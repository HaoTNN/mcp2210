#include <linux/init.h>
#include <linux/module.h>
#include <linux/usb.h>

#define DEVICE_VENDOR_ID 0x04d8
#define DEVICE_PRODUCT_ID 0x00de

static int mcp2210_probe(struct usb_interface *interface, const struct usb_device_id *id);
static void mcp2210_disconnect(struct usb_interface *interface);

struct mcp2210_usb {

};

static struct usb_device_id mcp2210_devices[] = {
	{ USB_DEVICE(DEVICE_VENDOR_ID, DEVICE_PRODUCT_ID) },
	{ }
};
MODULE_DEVICE_TABLE(usb, mcp2210_devices);

static struct usb_driver mcp2210_driver = {
	.name = "mcp2210",
	.id_table = mcp2210_devices,
	.probe = mcp2210_probe,
	.disconnect = mcp2210_disconnect,
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
	printk(KERN_INFO "MCP2210 plugged in: %04X:%04X\n", id->idVendor, id->idProduct);
	return 0;
}

static void mcp2210_disconnect(struct usb_interface *interface)
{
	printk(KERN_INFO "MCP2210 Disconnecting\n");
}

MODULE_LICENSE("GPL");

module_init(mcp2210_init);
module_exit(mcp2210_exit);
