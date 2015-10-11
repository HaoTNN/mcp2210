#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/usb.h>
#include <linux/mutex.h>

#include "mcp2210_cmd.h"

#define DEVICE_VENDOR_ID 0x04d8
#define DEVICE_PRODUCT_ID 0x00de

#define BUFFER_SIZE 64
#define MINOR_BASE 0

static DEFINE_MUTEX(disconnect_mutex);

static int mcp2210_probe(struct usb_interface *interface, const struct usb_device_id *id);
static int mcp2210_open(struct inode *inode, struct file *filp);
static ssize_t mcp2210_read(struct file *filp, char __user *buf, size_t count, loff_t *offp);
static ssize_t mcp2210_write(struct file *filp, const char __user *buf, size_t count, loff_t *offp);
static void mcp2210_disconnect(struct usb_interface *interface);
static void mcp2210_delete(struct kref *kref);
static int mcp2210_release(struct inode *inode, struct file *filp);
static struct mcp2210_usb *mcp2210_to_dev(struct kref *kref);

static void mcp2210_in_urb_complete(struct urb *urb);
static void mcp2210_out_urb_complete(struct urb *urb);

/*
 * Local data to be saved and used
 */
struct mcp2210_usb_endpoint {
	struct usb_host_endpoint *endpoint;			// actual usb endpoint data type
	struct urb *urb;							// 
	char *buffer;								// buffer to store/send data
	unsigned int direction;						// direction of this endpoint
};

struct mcp2210_usb {
	struct usb_device *dev;						// 
	struct usb_interface *interface;			//
	struct mcp2210_usb_endpoint in_endpoint;	// Endpoint struct to receive usb data
	struct mcp2210_usb_endpoint out_endpoint;	// Endpoint struct to send usb data
	bool int_on;								// Is the device still running? need it to determine whether or not to resubmit IN urb
	bool readable;								// Input data currently readable
	bool in_int_working;						//
	bool out_int_working;						//
	struct kref kref;							// Reference counter
	int minor;									// Minor number of device
	struct mutex io_mutex;						// Mutex lock
	spinlock_t io_spinlock;						// Spinlock for I/O
	char *test_buffer;							// TEMPORARY FOR TESTING;REMOVE LATER
};

/*
 * Driver structs
 */
static struct usb_device_id mcp2210_devices[] = {
	{ USB_DEVICE(DEVICE_VENDOR_ID, DEVICE_PRODUCT_ID) },
	{}
};
MODULE_DEVICE_TABLE(usb, mcp2210_devices);

static struct usb_driver mcp2210_driver = {
	.name = "mcp2210",
	.id_table = mcp2210_devices,
	.probe = mcp2210_probe,
	.disconnect = mcp2210_disconnect,
};

const struct file_operations mcp2210_fops = {
	.owner = THIS_MODULE,
	.open = mcp2210_open,
	.release = mcp2210_release,
	.read = mcp2210_read,
	.write = mcp2210_write,
};

struct usb_class_driver mcp2210_class = {
	.name = "mcp2210_usb2spi",
	.fops = &mcp2210_fops,
	.minor_base = MINOR_BASE,
};

/*
 * Driver functions
 */
static int mcp2210_init(void)
{
	int result;
	printk(KERN_INFO "Initializing MCP2210 driver..\n");
	result = usb_register(&mcp2210_driver);
	if (result)
	{
		printk(KERN_ERR "Unable to register usb device\n");
		return result;
	}
	printk(KERN_INFO "Initializing Complete\n");
	return 0;
}

static void mcp2210_exit(void)
{
	printk(KERN_INFO "Exiting MCP2210 driver..\n");
	usb_deregister(&mcp2210_driver);
	printk(KERN_INFO "Exit complete\n");
}

static int mcp2210_open(struct inode *inode, struct file *filp)
{
	struct mcp2210_usb *dev;
	struct usb_interface *interface;
	int subminor;
	int retval = 0;

	printk(KERN_INFO "mcp2210_open: opening..\n");

	subminor = iminor(inode);

	interface = usb_find_interface(&mcp2210_driver, subminor);
	if (!interface)
	{
		printk(KERN_ERR "Unable to locate device for minor %i\n", subminor);
		retval = -ENODEV;
		goto exit;
	}

	printk(KERN_INFO "mcp2210_open: Got interface\n");

	dev = usb_get_intfdata(interface);
	if (!dev)
	{
		printk(KERN_ERR "Unable to locate device\n");
		retval = -ENODEV;
		goto exit;
	}

	printk(KERN_INFO "mcp2210_open: Got dev\n");

	/*retval = usb_autopm_get_interface(interface);
	printk(KERN_INFO "mcp2210_open: retval after usb_autopm_get_interface: %i\n", retval);
	if (retval)
	{
		goto exit;
	}*/

	kref_get(&dev->kref);

	printk(KERN_INFO "mcp2210_open: inc kref and usb_autopm\n");

	retval = usb_submit_urb(dev->in_endpoint.urb, GFP_KERNEL);

	filp->private_data = dev;

exit:

	printk(KERN_INFO "mcp2210_open: exiting with value: %i\n", retval);

	return retval;
}

static int mcp2210_release(struct inode *inode, struct file *filp)
{
	struct mcp2210_usb *dev;

	printk(KERN_INFO "mcp2210_release: releasing..\n");

	dev = filp->private_data;
	if (dev == NULL)
	{
		return -ENODEV;
	}

	printk(KERN_INFO "mcp2210_release: dev found..\n");

/*
	mutex_lock(&dev->io_mutex);
	if (dev->interface)
	{
		usb_autopm_put_interface(dev->interface);
	}
	mutex_unlock(&dev->io_mutex);
*/

	kref_put(&dev->kref, mcp2210_delete);

	usb_kill_urb(dev->in_endpoint.urb);

	printk(KERN_INFO "mcp2210_release: exiting\n");


	return 0;
}

static void mcp2210_delete(struct kref *kref)
{
	printk(KERN_INFO "mcp2210_delete: deleting..\n");

	struct mcp2210_usb *dev = mcp2210_to_dev(kref);
	printk(KERN_INFO "mcp2210_delete: got dev from kref");

	kfree(dev->in_endpoint.buffer);
	kfree(dev->out_endpoint.buffer);

	usb_kill_urb(dev->in_endpoint.urb);

	usb_free_urb(dev->in_endpoint.urb);
	usb_free_urb(dev->out_endpoint.urb);


	kfree(dev->test_buffer);
	printk(KERN_INFO "mcp2210_delete: freed dev->test_buffer\n");

	kfree(dev);
	printk(KERN_INFO "mcp2210_delete: freed dev\n");

	printk(KERN_INFO "mcp2210_delete: delete complete\n");
}

static struct mcp2210_usb *mcp2210_to_dev(struct kref *kref)
{
	return container_of(kref, struct mcp2210_usb, kref);
}

static ssize_t mcp2210_read(struct file *filp, char __user *buf, size_t count, loff_t *offp)
{
	struct mcp2210_usb *dev;
	int retval = count;

	dev = filp->private_data;
	
	printk(KERN_INFO "mcp2210_read: reading..\n");

	spin_lock(&dev->io_spinlock);

	// Is device still plugged in?
	if (!dev->dev)
	{
		printk(KERN_ERR "mcp2210_read: udev is NULL!\n");
		retval = -ENODEV;
		goto unlock_exit;
	}

	// TODO: retry a read operation if it's currently unreadable?
	if (dev->readable)
	{
		retval = copy_to_user(buf, dev->in_endpoint.buffer, count);
		if (retval)
		{
			retval = -EFAULT;
			printk(KERN_ERR "mcp2210_read: could not read!\n");
			goto unlock_exit;
		}
	}
	else
	{
		retval = -EAGAIN;
		printk(KERN_INFO "mcp2210_read: device is busy!\n");
		goto unlock_exit;
	}

unlock_exit:
	printk(KERN_INFO "mcp2210_read: read complete\n");
	spin_unlock(&dev->io_spinlock);
	return retval;
}

static ssize_t mcp2210_write(struct file *filp, const char __user *buf, size_t count, loff_t *offp)
{
	struct mcp2210_usb *dev;
	int retval = count;

	dev = filp->private_data;

	printk(KERN_INFO "mcp2210_write: writing..\n");

	spin_lock(&dev->io_spinlock);

	// Is device still plugged in?
	if (!dev->dev)
	{
		printk(KERN_ERR "mcp2210_write: udev is NULL!\n");
		retval = -ENODEV;
		goto unlock_exit;
	}

	// Forcing size to be BUFFER_SIZE, which should be 64
	if (count != BUFFER_SIZE)
	{
		printk(KERN_ERR "mcp2210_write: count is not %i!\n", BUFFER_SIZE);
		goto unlock_exit;
	}

	// Everything looks good, let's try to get data from user now and send it off
	retval = copy_from_user(dev->out_endpoint.buffer, buf, count);
	if (retval)
	{
		printk(KERN_ERR "mcp2210_write: could not copy user data!\n");
		retval = -EFAULT;
		goto unlock_exit;
	}

/*
	printk(KERN_INFO "mcp2210_write: user input:\n");
	unsigned int i = 0;
	for (i; i < BUFFER_SIZE; ++i)
	{
		printk(KERN_INFO "0x%02hhx\n", dev->out_endpoint.buffer[i]);
	}
*/

	// Attempt to send command to our device here...
	retval = usb_submit_urb(dev->out_endpoint.urb, GFP_KERNEL);
	if (retval)
	{
		printk(KERN_ERR "mcp2210_write: could not submit urb successfully\n");
		goto unlock_exit;
	}
	retval = count;
	goto exit;

unlock_exit:
	spin_unlock(&dev->io_spinlock);
exit:
	return retval;
}

static int mcp2210_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
	int retval = -ENODEV;
	struct usb_device *udev;
	struct usb_host_interface *interface_desc;
	struct mcp2210_usb *dev;

	struct usb_host_endpoint *last_endpoint, *endpoint;

	printk(KERN_INFO "mcp2210_probe: MCP2210 plugged in: %04X:%04X\n", id->idVendor, id->idProduct);
	udev = interface_to_usbdev(interface);

	if (!udev)
	{
		printk(KERN_ERR "udev is NULL");
		return retval;
	}

	interface_desc = interface->cur_altsetting;

	dev = kzalloc(sizeof(struct mcp2210_usb), GFP_KERNEL);
	if (!dev)
	{
		printk(KERN_ERR "Error kzalloc\n");
		retval = -ENOMEM;
		goto exit;
	}

	dev->dev = udev;
	dev->interface = interface;

	// Setting up endpoints and urbs
	last_endpoint = &interface_desc->endpoint[interface_desc->desc.bNumEndpoints];
	for (endpoint = interface_desc->endpoint; endpoint != last_endpoint; ++endpoint)
	{
		// Is the endpoint of an interrupt type?
		if (!usb_endpoint_xfer_int(&endpoint->desc))
		{
			continue;
		}

		// Get direction of endpoint and proceed accordingly in setup of endpoints and urbs
		// Is endpoint's direction IN?
		if (!!usb_endpoint_dir_in(&endpoint->desc))
		{
			struct mcp2210_usb_endpoint *input_endpoint = &dev->in_endpoint;
			input_endpoint->buffer = kzalloc(BUFFER_SIZE, GFP_KERNEL);

			struct urb *urb = usb_alloc_urb(0, GFP_KERNEL);
			unsigned int pipeline = usb_rcvintpipe(udev, endpoint->desc.bEndpointAddress);

			input_endpoint->urb = urb;
			usb_fill_int_urb(urb, udev, pipeline, input_endpoint->buffer, BUFFER_SIZE, mcp2210_in_urb_complete, dev, endpoint->desc.bInterval);
			printk(KERN_INFO "mcp2210_probe: initialized input endpoint\n");
		}
		// It's OUT
		else
		{
			struct mcp2210_usb_endpoint *output_endpoint = &dev->out_endpoint;
			output_endpoint->buffer = kzalloc(BUFFER_SIZE, GFP_KERNEL);

			struct urb *urb = usb_alloc_urb(0, GFP_KERNEL);
			unsigned int pipeline = usb_sndintpipe(udev, endpoint->desc.bEndpointAddress);

			output_endpoint->urb = urb;
			usb_fill_int_urb(urb, udev, pipeline, output_endpoint->buffer, BUFFER_SIZE, mcp2210_out_urb_complete, dev, endpoint->desc.bInterval);
			printk(KERN_INFO "mcp2210_probe: initialized output endpoint\n");
		}
	}
	dev->int_on = true;
	dev->readable = false;
	dev->in_int_working = false;
	dev->out_int_working = false;
	kref_init(&dev->kref);
	dev->minor = interface->minor;
	mutex_init(&dev->io_mutex);
	spin_lock_init(&dev->io_spinlock);
	dev->test_buffer = kzalloc(BUFFER_SIZE, GFP_KERNEL);

	usb_set_intfdata(interface, dev);
	
	retval = usb_register_dev(interface, &mcp2210_class);

	if (retval)
	{
		printk(KERN_ERR "Unable to register usb device\n");
		goto error;
	}


	printk(KERN_INFO "mcp2210_probe: probe complete\n");

	return 0;

error:
	kfree(dev->test_buffer);
	kfree(dev);

exit:
	return retval;
}

static void mcp2210_in_urb_complete(struct urb *urb)
{
	struct mcp2210_usb *dev = urb->context;
	int retval;

	printk(KERN_INFO "mcp2210_in_urb_complete: completed, status: %i", urb->status);

	if (urb->status)
	{
		if (urb->status == -ENOENT || urb->status == -ECONNRESET || urb->status == -ESHUTDOWN)
		{
			return;
		}
		else
		{
			printk(KERN_ERR "mcp2210_in_urb_complete: non-zero urb status: %d\n", urb->status);
			return;
		}
	}

	dev->readable = true;
	
	unsigned int i;
	for (i = 0; i < 32; ++i)
	{
		printk(KERN_INFO "mcp2210_in_urb_complete: %i: 0x%02hhX\n", i, dev->in_endpoint.buffer[i]);
	}
	
	//printk(KERN_INFO "mcp2210_in_urb_complete: 0x%02X\n", dev->in_endpoint.buffer);
	printk(KERN_INFO "mcp2210_in_urb_complete: urb->actual_length: %d\n", urb->actual_length);

	// Resubmitting
	if (dev->int_on)
	{
		retval = usb_submit_urb(urb, GFP_ATOMIC);
		if (retval)
		{
			printk(KERN_ERR "mcp2210_in_urb_complete: resubmitting urb failed\n");
			dev->int_on = false;
		}
	}
	spin_unlock(&dev->io_spinlock);
}

static void mcp2210_out_urb_complete(struct urb *urb)
{
	struct mcp2210_usb *dev = urb->context;
	int retval;

	printk(KERN_INFO "mcp2210_out_urb_complete: completed, status: %i", urb->status);

	if (urb->status)
	{
		if (urb->status == -ENOENT || urb->status == -ECONNRESET || urb->status == -ESHUTDOWN)
		{
			return;
		}
		else
		{
			printk(KERN_ERR "mcp2210_out_urb_complete: non-zero urb status: %d\n", urb->status);
			return;
		}
	}

	dev->readable = true;

	/*
	for (i; i < BUFFER_SIZE; ++i)
	{
		printk(KERN_INFO "mcp2210_out_urb_complete: %i: 0x%02X\n", i, dev->out_endpoint.buffer[i]);
	}
	printk(KERN_INFO "mcp2210_out_urb_complete: 0x%02X\n", dev->out_endpoint.buffer);
	*/
	printk(KERN_INFO "mcp2210_out_urb_complete: urb->actual_length: %d\n", urb->actual_length);
}

static void mcp2210_disconnect(struct usb_interface *interface)
{
	printk(KERN_INFO "MCP2210 Disconnecting\n");

	// Need to clear up allocated data
	struct mcp2210_usb *dev = usb_get_intfdata(interface);
	usb_set_intfdata(interface, NULL);
	printk(KERN_INFO "mcp2210_disconnect: interface data set to NULL\n");

	kref_put(&dev->kref, mcp2210_delete);
	printk(KERN_INFO "mcp2210_disconnect: decremented dev->kref\n");

	usb_deregister_dev(interface, &mcp2210_class);
	printk(KERN_INFO "mcp2210_disconnect: deregistered usb\n");

	printk(KERN_INFO "Disconnect complete\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Hao Nguyen");

module_init(mcp2210_init);
module_exit(mcp2210_exit);
