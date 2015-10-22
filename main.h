#ifndef _MCP2210_H
#define _MCP2210_H

#define DEFAULT_DEVICE_PATH "/dev/mcp2210_usb2spi"
#define CMD_SIZE 64
#define MAX_ARGUMENT_CNT 64
#define MAX_ARGUMENT_LEN 32

/* TODO: Redesign command structure... a bit messy atm ._.
 * TODO: Use C++?
 * TODO: main.h doesn't make much sense as a name
 * TODO: nvram and ram command syntax is inconsistent; i.e,
 * ram gpio_val set vs.
 * nvram set power_up_defaults
 *
 * TODO: Have the option of setting only 1 option at a time; e.g
 * for configuring chip settings, I may only want to change byte index 8,
 * instead of having to change all bytes
 */
struct command {
	char *name;
	const struct command *sub_cmd;
	int (*func)(int fd, char user_input[MAX_ARGUMENT_CNT][MAX_ARGUMENT_LEN], size_t user_count, char *save_buffer, size_t save_count, int index);
};

int ram_set_gpio_chip(int fd, char user_input[MAX_ARGUMENT_CNT][MAX_ARGUMENT_LEN], size_t user_count, char *save_buffer, size_t save_count, int index);
int ram_get_gpio_chip(int fd, char user_input[MAX_ARGUMENT_CNT][MAX_ARGUMENT_LEN], size_t user_count, char *save_buffer, size_t save_count, int index);
int ram_set_gpio_dir(int fd, char user_input[MAX_ARGUMENT_CNT][MAX_ARGUMENT_LEN], size_t user_count, char *save_buffer, size_t save_count, int index);
int ram_get_gpio_dir(int fd, char user_input[MAX_ARGUMENT_CNT][MAX_ARGUMENT_LEN], size_t user_count, char *save_buffer, size_t save_count, int index);
int ram_set_gpio_val(int fd, char user_input[MAX_ARGUMENT_CNT][MAX_ARGUMENT_LEN], size_t user_count, char *save_buffer, size_t save_count, int index);
int ram_get_gpio_val(int fd, char user_input[MAX_ARGUMENT_CNT][MAX_ARGUMENT_LEN], size_t user_count, char *save_buffer, size_t save_count, int index);
int ram_set_spi(int fd, char user_input[MAX_ARGUMENT_CNT][MAX_ARGUMENT_LEN], size_t user_Count, char *save_buffer, size_t save_count, int index);
int ram_get_spi(int fd, char user_input[MAX_ARGUMENT_CNT][MAX_ARGUMENT_LEN], size_t user_Count, char *save_buffer, size_t save_count, int index);

int nvram_set_power_up_defaults(int fd, char user_input[MAX_ARGUMENT_CNT][MAX_ARGUMENT_LEN], size_t user_count, char *save_buffer, size_t save_count, int index);
int nvram_set_spi_power_up_settings(int fd, char user_input[MAX_ARGUMENT_CNT][MAX_ARGUMENT_LEN], size_t user_count, char *save_buffer, size_t save_count, int index);
int nvram_set_usb_power_up_params(int fd, char user_input[MAX_ARGUMENT_CNT][MAX_ARGUMENT_LEN], size_t user_count, char *save_buffer, size_t save_count, int index);
int nvram_set_usb_manufacturer_name(int fd, char user_input[MAX_ARGUMENT_CNT][MAX_ARGUMENT_LEN], size_t user_count, char *save_buffer, size_t save_count, int index);
int nvram_set_usb_name(int fd, char user_input[MAX_ARGUMENT_CNT][MAX_ARGUMENT_LEN], size_t user_count, char *save_buffer, size_t save_count, int index);

int nvram_get_power_up_settings(int fd, char user_input[MAX_ARGUMENT_CNT][MAX_ARGUMENT_LEN], size_t user_count, char *save_buffer, size_t save_count, int index);
int nvram_get_spi_power_up_settings(int fd, char user_input[MAX_ARGUMENT_CNT][MAX_ARGUMENT_LEN], size_t user_count, char *save_buffer, size_t save_count, int index);
int nvram_get_usb_power_up_params(int fd, char user_input[MAX_ARGUMENT_CNT][MAX_ARGUMENT_LEN], size_t user_count, char *save_buffer, size_t save_count, int index);
int nvram_get_usb_manufacturer_name(int fd, char user_input[MAX_ARGUMENT_CNT][MAX_ARGUMENT_LEN], size_t user_count, char *save_buffer, size_t save_count, int index);
int nvram_get_usb_name(int fd, char user_input[MAX_ARGUMENT_CNT][MAX_ARGUMENT_LEN], size_t user_count, char *save_buffer, size_t save_count, int index);

int nvram_send_pw(int fd, char user_input[MAX_ARGUMENT_CNT][MAX_ARGUMENT_LEN], size_t user_count, char *save_buffer, size_t save_count, int index);

int spi_transfer_data(int fd, char user_input[MAX_ARGUMENT_CNT][MAX_ARGUMENT_LEN], size_t user_count, char *save_buffer, size_t save_count, int index);
int spi_cancel_transfer(int fd, char user_input[MAX_ARGUMENT_CNT][MAX_ARGUMENT_LEN], size_t user_count, char *save_buffer, size_t save_count, int index);

int chip_status(int fd, char user_input[MAX_ARGUMENT_CNT][MAX_ARGUMENT_LEN], size_t user_count, char *save_buffer, size_t save_count, int index);

int not_yet_implemented(int fd, char user_input[MAX_ARGUMENT_CNT][MAX_ARGUMENT_LEN], size_t user_count, char *save_buffer, size_t save_count, int index);

static const struct command gpio_chip[] = {
	{
		.name = "set",
		.func = ram_set_gpio_chip,
	},
	{
		.name = "get",
		.func = ram_get_gpio_chip,
	}, {}
};

static const struct command gpio_dir[] = {
	{
		.name = "set",
		.func = ram_set_gpio_dir,
	},
	{
		.name = "get",
		.func = ram_get_gpio_dir,
	}, {}
};

static const struct command gpio_val[] = {
	{
		.name = "set",
		.func = ram_set_gpio_val,
	},
	{
		.name = "get",
		.func = ram_get_gpio_val,
	}, {}
};

static const struct command ram_spi[] = {
	{
		.name = "set",
		.func = ram_set_spi,
	},
	{
		.name = "get",
		.func = ram_get_spi,
	}, {}
};

static const struct command nvram_set[] = {
	{
		.name = "power_up_defaults",
		.func = nvram_set_power_up_defaults,
	},
	{
		.name = "spi_power_up_settings",
		.func = nvram_set_spi_power_up_settings,
	},
	{
		.name = "usb_power_up_params",
		.func = nvram_set_usb_power_up_params,
	},
	{
		.name = "usb_manufacturer_name",
		.func = nvram_set_usb_manufacturer_name,
	},
	{
		.name = "usb_name",
		.func = nvram_set_usb_name,
	}, {}
};

static const struct command nvram_get[] = {
	{
		.name = "spi_power_up_settings",
		.func = nvram_get_spi_power_up_settings,
	},
	{
		.name = "power_up_settings",
		.func = nvram_get_power_up_settings,
	},
	{
		.name = "usb_power_up_params",
		.func = nvram_get_usb_power_up_params,
	},
	{
		.name = "usb_manufacturer_name",
		.func = nvram_get_usb_manufacturer_name,
	},
	{
		.name = "usb_name",
		.func = nvram_get_usb_name,
	}, {}
};

static const struct command ram[] = {
	{
		.name = "gpio_chip",
		.sub_cmd = gpio_chip,
	},
	{
		.name = "gpio_dir",
		.sub_cmd = gpio_dir,
	},
	{
		.name = "gpio_val",
		.sub_cmd = gpio_val,
	},
	{
		.name = "spi",
		.sub_cmd = ram_spi,
	}, {}
};

static const struct command nvram[] = {
	{
		.name = "set",
		.sub_cmd = nvram_set,
	},
	{
		.name = "get",
		.sub_cmd = nvram_get,
	},
	{
		.name = "send_pw",
		.func = not_yet_implemented,
	}, {}
};

static const struct command spi[] = {
	{
		.name = "transfer_data",
		.func = spi_transfer_data,
	},
	{
		.name = "cancel_transfer",
		.func = spi_cancel_transfer,
	}, {}
};

static const struct command categories[] = {
	{
		.name = "nvram",
		.sub_cmd = nvram,
	},
	{
		.name = "ram",
		.sub_cmd = ram,
	}, 
	{
		.name = "chip_status",
		.func = chip_status,
	},{}
};
#endif
