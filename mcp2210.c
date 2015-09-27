#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/usb.h>
#include <linux/mutex.h>

#define DEVICE_VENDOR_ID 0x04d8
#define DEVICE_PRODUCT_ID 0x00de

#define BUFFER_SIZE 64
#define MINOR_BASE 0

static DEFINE_MUTEX(disconnect_mutex);

static int mcp2210_probe(struct usb_interface *interface, const struct usb_device_id *id);
static void mcp2210_disconnect(struct usb_interface *interface);
static void mcp2210_urb_complete(struct urb *urb);

struct mcp2210_usb_endpoint {
	struct usb_host_endpoint *endpoint;
	struct urb *urb;
	char *buffer;
	unsigned int direction;
};

struct mcp2210_usb {
	struct usb_device *dev;
	struct usb_interface *interface;
	struct mcp2210_usb_endpoint in_endpoint;
	struct mcp2210_usb_endpoint out_endpoint;
	int minor;
};

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
	.read = NULL,
	.write = NULL,
};

struct usb_class_driver mcp2210_class = {
	.name = "mcp2210_usb2spi",
	.fops = &mcp2210_fops,
	.minor_base = MINOR_BASE,
};

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

static int mcp2210_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
	int retval = -ENODEV;
	struct usb_device *udev;
	struct usb_host_interface *interface_desc;
	struct mcp2210_usb *dev;

	//struct usb_host_endpoint *last_endpoint, *endpoint;

	printk(KERN_INFO "MCP2210 plugged in: %04X:%04X\n", id->idVendor, id->idProduct);
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

	dev->interface = interface;
	dev->dev = udev;

	// Setting up endpoints and urbs
	/*
	last_endpoint = &interface_desc->endpoint[interface_desc->desc.bNumEndpoints];
	for (endpoint = interface_desc->endpoint; endpoint != last_endpoint; ++endpoint)
	{
		// Is the endpoint of an interrupt type?
		if (!usb_endpoint_xfer_int(&endpoint->desc))
		{
			continue;
		}

		// Get direction of endpoint and proceed accordingly in setup of endpoints and urbs
		if (!!usb_endpoint_dir_in(&endpoint->desc))
		{
			struct mcp2210_usb_endpoint *input_endpoint = &dev->in_endpoint;
			input_endpoint->buffer = kzalloc(BUFFER_SIZE, GFP_KERNEL);

			struct urb *urb = usb_alloc_urb(0, GFP_KERNEL);
			unsigned int pipeline = usb_rcvintpipe(udev, endpoint->desc.bEndpointAddress);

			input_endpoint->urb = urb;
			usb_fill_int_urb(urb, udev, pipeline, input_endpoint->buffer, BUFFER_SIZE, mcp2210_urb_complete, udev, endpoint->desc.bInterval);
		}
		else
		{
			struct mcp2210_usb_endpoint *output_endpoint = &dev->out_endpoint;
			output_endpoint->buffer = kzalloc(BUFFER_SIZE, GFP_KERNEL);
			struct urb *urb = usb_alloc_urb(0, GFP_KERNEL);
			unsigned int pipeline = usb_rcvintpipe(udev, endpoint->desc.bEndpointAddress);

			output_endpoint->urb = urb;
			usb_fill_int_urb(urb, udev, pipeline, output_endpoint->buffer, BUFFER_SIZE, mcp2210_urb_complete, udev, endpoint->desc.bInterval);
		}
	}*/

	usb_set_intfdata(interface, dev);
	
	retval = usb_register_dev(interface, &mcp2210_class);

	printk(KERN_INFO "Completing registration\n");
	if (retval)
	{
		printk(KERN_ERR "Unable to register usb device\n");
		goto error;
	}

	dev->minor = interface->minor;

	return 0;

error:
	kfree(dev);

exit:
	return retval;
}

static void mcp2210_urb_complete(struct urb *urb)
{
	printk(KERN_INFO "Urb completed, status: %i", urb->status);
}

static void mcp2210_disconnect(struct usb_interface *interface)
{
	printk(KERN_INFO "MCP2210 Disconnecting\n");

	mutex_lock(&disconnect_mutex);

	// Need to clear up allocated data
	struct mcp2210_usb *dev = usb_get_intfdata(interface);
	usb_set_intfdata(interface, NULL);

	kfree(dev);
	usb_deregister_dev(interface, &mcp2210_class);

	mutex_unlock(&disconnect_mutex);

	printk(KERN_INFO "Disconnect complete\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Hao Nguyen");

module_init(mcp2210_init);
module_exit(mcp2210_exit);
