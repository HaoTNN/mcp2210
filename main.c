#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "main.h"
#include "cmd_helper_functions.c"
#include "nrf24l01.h"

#define QUIT_CMD "q"
#define GET_GPIO_DIR_CMD "get_gpio_dir"
#define SET_GPIO_DIR_CMD "set_gpio_dir"
#define SET_GPIO_VAL_CMD "set_gpio_val"

// TODO: See main.h

int send_cmd(int fd, const char* cmd_buffer, char *save_buffer, size_t cmd_count, size_t save_count);
int command_result(char return_code);
void print_as_binary(char c);
char process_hex(const char* src);

int not_yet_implemented(int fd, char user_input[MAX_ARGUMENT_CNT][MAX_ARGUMENT_LEN], size_t user_count, char *save_buffer, size_t save_count, int index)
{
	fprintf(stdout, "Command not implemented\n");
	return 0;
}

int ram_set_gpio_chip(int fd, char user_input[MAX_ARGUMENT_CNT][MAX_ARGUMENT_LEN], size_t user_count, char *save_buffer, size_t save_count, int index)
{
	char cmd_buffer[64] = "";
	cmd_buffer[0] = 0x21;

	unsigned int i;
	unsigned int j;
	// Password settings are from byte index 18 to 26. We're gonna ignore that for now
	for (i = 4, j = index; i < 18; ++i, ++j)
	{
		cmd_buffer[i] = process_hex(user_input[j]);
	}

	int status = send_cmd(fd, cmd_buffer, save_buffer, sizeof(cmd_buffer), save_count);
	if (status)
	{
		return status;
	}
	
	return command_result(save_buffer[1]);
}

int ram_get_gpio_chip(int fd, char user_input[MAX_ARGUMENT_CNT][MAX_ARGUMENT_LEN], size_t user_count, char *save_buffer, size_t save_count, int index)
{
	char cmd_buffer[64] = "";
	cmd_buffer[0] = 0x20;

	int status = send_cmd(fd, cmd_buffer, save_buffer, sizeof(cmd_buffer), save_count);
	if (status)
	{
		return status;
	}

	if(!command_result(save_buffer[1]))
	{
		fprintf(stdout, "Values:\n");
		unsigned int i;
		for (i = 4; i < 19; ++i)
		{
			fprintf(stdout, "%i: ", i);
			print_as_binary(save_buffer[i]);
			fprintf(stdout, "\n");
		}
		return 0;
	}
	else
	{
		return -1;
	}
}

int ram_set_gpio_dir(int fd, char user_input[MAX_ARGUMENT_CNT][MAX_ARGUMENT_LEN], size_t user_count, char *save_buffer, size_t save_count, int index)
{
	char cmd_buffer[64] = "";
	cmd_buffer[0] = 0x32;

	unsigned int i;
	unsigned int j;
	for (i = 4, j = index; i < 6; ++i, ++j)
	{
		cmd_buffer[i] = process_hex(user_input[j]);
	}

	int status = send_cmd(fd, cmd_buffer, save_buffer, sizeof(cmd_buffer), save_count);
	if (status)
	{
		return status;
	}
	
	return command_result(save_buffer[1]);
}

int ram_get_gpio_dir(int fd, char user_input[MAX_ARGUMENT_CNT][MAX_ARGUMENT_LEN], size_t user_count, char *save_buffer, size_t save_count, int index)
{
	char cmd_buffer[64] = "";
	cmd_buffer[0] = 0x33;

	int status = send_cmd(fd, cmd_buffer, save_buffer, sizeof(cmd_buffer), save_count);
	if (status)
	{
		return status;
	}

	if(!command_result(save_buffer[1]))
	{
		fprintf(stdout, "Values:\n");
		unsigned int i;
		for (i = 4; i < 6; ++i)
		{
			fprintf(stdout, "%i: ", i);
			print_as_binary(save_buffer[i]);
			fprintf(stdout, "\n");
		}
		return 0;
	}
	else
	{
		return -1;
	}
}

int ram_set_gpio_val(int fd, char user_input[MAX_ARGUMENT_CNT][MAX_ARGUMENT_LEN], size_t user_count, char *save_buffer, size_t save_count, int index)
{
	char cmd_buffer[64] = "";
	cmd_buffer[0] = 0x30;

	unsigned int i;
	unsigned int j;
	for (i = 4, j = index; i < 6; ++i, ++j)
	{
		cmd_buffer[i] = process_hex(user_input[j]);
	}

	int status = send_cmd(fd, cmd_buffer, save_buffer, sizeof(cmd_buffer), save_count);
	if (status)
	{
		return status;
	}
	
	if(!command_result(save_buffer[1]))
	{
		fprintf(stdout, "Values:\n");
		unsigned int i;
		for (i = 4; i < 6; ++i)
		{
			fprintf(stdout, "%i: ", i);
			print_as_binary(save_buffer[i]);
			fprintf(stdout, "\n");
		}
		return 0;
	}
	else
	{
		return -1;
	}
}

int ram_get_gpio_val(int fd, char user_input[MAX_ARGUMENT_CNT][MAX_ARGUMENT_LEN], size_t user_count, char *save_buffer, size_t save_count, int index)
{
	char cmd_buffer[64] = "";
	cmd_buffer[0] = 0x31;

	int status = send_cmd(fd, cmd_buffer, save_buffer, sizeof(cmd_buffer), save_count);
	if (status)
	{
		return status;
	}
	
	if(!command_result(save_buffer[1]))
	{
		fprintf(stdout, "Values:\n");
		unsigned int i;
		for (i = 4; i < 6; ++i)
		{
			fprintf(stdout, "%i: ", i);
			print_as_binary(save_buffer[i]);
			fprintf(stdout, "\n");
		}
		return 0;
	}
	else
	{
		return -1;
	}
}

int ram_set_spi(int fd, char user_input[MAX_ARGUMENT_CNT][MAX_ARGUMENT_LEN], size_t user_count, char *save_buffer, size_t save_count, int index)
{
	char cmd_buffer[64] = "";
	cmd_buffer[0] = 0x40;

	unsigned int i;
	unsigned int j;
	for (i = 4, j = index; i < 21; ++i, ++j)
	{
		cmd_buffer[i] = process_hex(user_input[j]);
	}

	int status = send_cmd(fd, cmd_buffer, save_buffer, sizeof(cmd_buffer), save_count);
	if (status)
	{
		return status;
	}

	if (!command_result(save_buffer[1]))
	{
		fprintf(stdout, "Values:\n");
		unsigned int i;
		for (i = 4; i < 21; ++i)
		{
			fprintf(stdout, "%i: ", i);
			print_as_binary(save_buffer[i]);
			fprintf(stdout, "\n");
		}
		return 0;
	}
	else
	{
		return -1;		
	}
}

int ram_get_spi(int fd, char user_input[MAX_ARGUMENT_CNT][MAX_ARGUMENT_LEN], size_t user_count, char *save_buffer, size_t save_count, int index)
{
	char cmd_buffer[64] = "";
	cmd_buffer[0] = 0x41;

	int status = send_cmd(fd, cmd_buffer, save_buffer, sizeof(cmd_buffer), save_count);
	if (status)
	{
		return status;
	}

	if (!command_result(save_buffer[1]))
	{
		fprintf(stdout, "Values:\n");
		unsigned int i;
		for (i = 4; i < 21; ++i)
		{
			fprintf(stdout, "%i: ", i);
			print_as_binary(save_buffer[i]);
			fprintf(stdout, "\n");
		}
		return 0;
	}
	else
	{
		return -1;
	}
}

int nvram_set_power_up_defaults(int fd, char user_input[MAX_ARGUMENT_CNT][MAX_ARGUMENT_LEN], size_t user_count, char *save_buffer, size_t save_count, int index)
{
	char cmd_buffer[64] = "";
	cmd_buffer[0] = 0x60;
	cmd_buffer[1] = 0x20;

	unsigned int i;
	unsigned int j;
	// Password settings are from byte index 18 to 26. We're gonna ignore that for now
	for (i = 4, j = index; i < 18; ++i, ++j)
	{
		cmd_buffer[i] = process_hex(user_input[j]);
	}

	int status = send_cmd(fd, cmd_buffer, save_buffer, sizeof(cmd_buffer), save_count);
	if (status)
	{
		return status;
	}
	
	return command_result(save_buffer[1]);
}

int nvram_set_spi_power_up_settings(int fd, char user_input[MAX_ARGUMENT_CNT][MAX_ARGUMENT_LEN], size_t user_count, char *save_buffer, size_t save_count, int index)
{
	char cmd_buffer[64] = "";
	cmd_buffer[0] = 0x60;
	cmd_buffer[1] = 0x10;
	unsigned int i;
	unsigned int j;
	for (i = 4, j = index; i < 21; ++i, ++j)
	{
		cmd_buffer[i] = process_hex(user_input[j]);
	}

	int status = send_cmd(fd, cmd_buffer, save_buffer, sizeof(cmd_buffer), save_count);
	if (status)
	{
		return status;
	}

	return command_result(save_buffer[1]);
}

int nvram_set_usb_power_up_params(int fd, char user_input[MAX_ARGUMENT_CNT][MAX_ARGUMENT_LEN], size_t user_count, char *save_buffer, size_t save_count, int index)
{
	char cmd_buffer[64] = "";
	cmd_buffer[0] = 0x60;
	cmd_buffer[1] = 0x30;
	unsigned int i;
	unsigned int j;
	for (i = 4, j = index; i < 10; ++i, ++j)
	{
		cmd_buffer[i] = process_hex(user_input[j]);
	}

	int status = send_cmd(fd, cmd_buffer, save_buffer, sizeof(cmd_buffer), save_count);
	if (status)
	{
		return status;
	}

	return command_result(save_buffer[1]);
}

int nvram_set_usb_manufacturer_name(int fd, char user_input[MAX_ARGUMENT_CNT][MAX_ARGUMENT_LEN], size_t user_count, char *save_buffer, size_t save_count, int index)
{
	char cmd_buffer[64] = "";
	cmd_buffer[0] = 0x60;
	cmd_buffer[1] = 0x50;
	unsigned int i;
	unsigned int j;
	for (i = 4, j = index; i < 64; ++i, ++j)
	{
		cmd_buffer[i] = process_hex(user_input[j]);
	}
	// Specs indicate to fill index 5 with 0x03
	cmd_buffer[5] = 0x03;

	int status = send_cmd(fd, cmd_buffer, save_buffer, sizeof(cmd_buffer), save_count);
	if (status)
	{
		return status;
	}

	return command_result(save_buffer[1]);
}

int nvram_set_usb_name(int fd, char user_input[MAX_ARGUMENT_CNT][MAX_ARGUMENT_LEN], size_t user_count, char *save_buffer, size_t save_count, int index)
{
	char cmd_buffer[64] = "";
	cmd_buffer[0] = 0x60;
	cmd_buffer[1] = 0x40;
	unsigned int i;
	unsigned int j;
	for (i = 4, j = index; i < 64; ++i, ++j)
	{
		cmd_buffer[i] = process_hex(user_input[j]);
	}
	// Specs indicate to fill index 5 with 0x03
	cmd_buffer[5] = 0x03;

	int status = send_cmd(fd, cmd_buffer, save_buffer, sizeof(cmd_buffer), save_count);
	if (status)
	{
		return status;
	}

	return command_result(save_buffer[1]);
}

int nvram_get_spi_power_up_settings(int fd, char user_input[MAX_ARGUMENT_CNT][MAX_ARGUMENT_LEN], size_t user_count, char *save_buffer, size_t save_count, int index)
{
	char cmd_buffer[64] = "";
	cmd_buffer[0] = 0x61;
	cmd_buffer[1] = 0x10;

	int status = send_cmd(fd, cmd_buffer, save_buffer, sizeof(cmd_buffer), save_count);
	if (status)
	{
		return status;
	}

	if(!command_result(save_buffer[1]))
	{
		fprintf(stdout, "Values:\n");
		unsigned int i;
		for (i = 4; i < 21; ++i)
		{
			fprintf(stdout, "%i: ", i);
			print_as_binary(save_buffer[i]);
			fprintf(stdout, "\n");
		}
		return 0;
	}
	else
	{
		return -1;
	}
}

int nvram_get_power_up_settings(int fd, char user_input[MAX_ARGUMENT_CNT][MAX_ARGUMENT_LEN], size_t user_count, char *save_buffer, size_t save_count, int index)
{
	char cmd_buffer[64] = "";
	cmd_buffer[0] = 0x61;
	cmd_buffer[1] = 0x20;

	int status = send_cmd(fd, cmd_buffer, save_buffer, sizeof(cmd_buffer), save_count);
	if (status)
	{
		return status;
	}

	if(!command_result(save_buffer[1]))
	{
		fprintf(stdout, "Values:\n");
		unsigned int i;
		for (i = 4; i < 19; ++i)
		{
			fprintf(stdout, "%i: ", i);
			print_as_binary(save_buffer[i]);
			fprintf(stdout, "\n");
		}
		return 0;
	}
	else
	{
		return -1;
	}
}
int nvram_get_usb_power_up_params(int fd, char user_input[MAX_ARGUMENT_CNT][MAX_ARGUMENT_LEN], size_t user_count, char *save_buffer, size_t save_count, int index)
{
	char cmd_buffer[64] = "";
	cmd_buffer[0] = 0x61;
	cmd_buffer[1] = 0x30;

	int status = send_cmd(fd, cmd_buffer, save_buffer, sizeof(cmd_buffer), save_count);
	if (status)
	{
		return status;
	}

	if(!command_result(save_buffer[1]))
	{
		fprintf(stdout, "Values:\n");
		unsigned int i;
		for (i = 12; i < 16; ++i)
		{
			fprintf(stdout, "%i: ", i);
			print_as_binary(save_buffer[i]);
			fprintf(stdout, "\n");
		}
		fprintf(stdout, "29: ");
		print_as_binary(save_buffer[29]);
		fprintf(stdout, "\n");
		fprintf(stdout, "30: ");
		print_as_binary(save_buffer[30]);
		fprintf(stdout, "\n");
		return 0;
	}
	else
	{
		return -1;
	}

}

int nvram_get_usb_manufacturer_name(int fd, char user_input[MAX_ARGUMENT_CNT][MAX_ARGUMENT_LEN], size_t user_count, char *save_buffer, size_t save_count, int index)
{
	char cmd_buffer[64] = "";
	cmd_buffer[0] = 0x61;
	cmd_buffer[1] = 0x50;

	int status = send_cmd(fd, cmd_buffer, save_buffer, sizeof(cmd_buffer), save_count);
	if (status)
	{
		return status;
	}

	if(!command_result(save_buffer[1]))
	{
		fprintf(stdout, "Values:\n");
		unsigned int i;
		for (i = 4; i < 64; ++i)
		{
			fprintf(stdout, "%i: ", i);
			print_as_binary(save_buffer[i]);
			fprintf(stdout, "\n");
		}
		return 0;
	}
	else
	{
		return -1;
	}
}

int nvram_get_usb_name(int fd, char user_input[MAX_ARGUMENT_CNT][MAX_ARGUMENT_LEN], size_t user_count, char *save_buffer, size_t save_count, int index)
{
	char cmd_buffer[64] = "";
	cmd_buffer[0] = 0x61;
	cmd_buffer[1] = 0x40;

	int status = send_cmd(fd, cmd_buffer, save_buffer, sizeof(cmd_buffer), save_count);
	if (status)
	{
		return status;
	}

	if (!command_result(save_buffer[1]))
	{
		fprintf(stdout, "Values:\n");
		unsigned int i;
		for (i = 4; i < 64; ++i)
		{
			fprintf(stdout, "%i: ", i);
			print_as_binary(save_buffer[i]);
			fprintf(stdout, "\n");
		}
		return 0;
	}
	else
	{
		return -1;
	}
}

int spi_transfer_data(int fd, char user_input[MAX_ARGUMENT_CNT][MAX_ARGUMENT_LEN], size_t user_count, char *save_buffer, size_t save_count, int index)
{
	char cmd_buffer[64] = "";
	cmd_buffer[0] = 0x42;
	cmd_buffer[1] = process_hex(user_input[index]);

	unsigned int i;
	unsigned int j;
	for (i = 4, j = index+1; i < 64; ++i, ++j)
	{
		cmd_buffer[i] = process_hex(user_input[j]);
	}

	int status = send_cmd(fd, cmd_buffer, save_buffer, sizeof(cmd_buffer), save_count);
	if (status)
	{
		return status;
	}

	if (!command_result(save_buffer[1]))
	{

		fprintf(stdout, "Values:\n");
		unsigned int i;
		for (i = 2; i < 64; ++i)
		{
			fprintf(stdout, "%i: ", i);
			print_as_binary(save_buffer[i]);
			fprintf(stdout, "\n");
		}
		return 0;
	}
	else
	{
		return -1;
	}
}

int spi_cancel_transfer(int fd, char user_input[MAX_ARGUMENT_CNT][MAX_ARGUMENT_LEN], size_t user_count, char *save_buffer, size_t save_count, int index)
{
	char cmd_buffer[64] = "";
	cmd_buffer[0] = 0x11;

	int status = send_cmd(fd, cmd_buffer, save_buffer, sizeof(cmd_buffer), save_count);
	if (status)
	{
		return status;
	}

	if (!command_result(save_buffer[1]))
	{

		fprintf(stdout, "Values:\n");
		unsigned int i;
		for (i = 2; i < 6; ++i)
		{
			fprintf(stdout, "%i: ", i);
			print_as_binary(save_buffer[i]);
			fprintf(stdout, "\n");
		}
		return 0;
	}
	else
	{
		return -1;
	}
}

int chip_status(int fd, char user_input[MAX_ARGUMENT_CNT][MAX_ARGUMENT_LEN], size_t user_count, char *save_buffer, size_t save_count, int index)
{
	char cmd_buffer[64] = "";
	cmd_buffer[0] = 0x10;

	int status = send_cmd(fd, cmd_buffer, save_buffer, sizeof(cmd_buffer), save_count);
	if (status)
	{
		return status;
	}

	if (!command_result(save_buffer[1]))
	{
		fprintf(stdout, "Values:\n");
		unsigned int i;
		for (i = 2; i < 6; ++i)
		{
			fprintf(stdout, "%i: ", i);
			print_as_binary(save_buffer[i]);
			fprintf(stdout, "\n");
		}
		return 0;
	}
	else
	{
		return -1;
	}
}

int main(int argc, char *argv[])
{
	int fd;
	char *dev = DEFAULT_DEVICE_PATH;
	char cmd_buffer[CMD_SIZE] = "";
	char save_buffer[CMD_SIZE] = "";
	char input[MAX_ARGUMENT_CNT][MAX_ARGUMENT_LEN];

	printf("Starting test program..\n");

	printf("Opening %s\n", DEFAULT_DEVICE_PATH);
	fd = open(dev, O_RDWR);

	if (fd < 0)
	{
		printf("Could not open file, error: %i\n", errno);
		return -1;
	}

	char input_buffer[128];
	fprintf(stdout, "Welcome to the MCP2210 Test Application. Type 'q' to quit\n");
	while (1)
	{
		// Zero out command buffer before using it!
		memset(cmd_buffer, 0, sizeof(cmd_buffer));
		int status = 0;
		printf("> ");

		// Getting user input
		fgets(input_buffer, sizeof(input_buffer), stdin);

		// Getting rid of new line, then begin tokenizing
		if (input_buffer[strlen(input_buffer)-1] == '\n')
		{
			input_buffer[strlen(input_buffer)-1] = '\0';
		}
		char *tokenized_input = strtok(input_buffer, " ");

		// Tokenizing input and putting it into an array we'll process
		unsigned int i;
		for (i = 0; i < MAX_ARGUMENT_CNT; ++i)
		{
			if (tokenized_input)
			{
				// If length of an input word is too long, don't do anything
				if (strlen(tokenized_input) > MAX_ARGUMENT_LEN)
				{
					fprintf(stdout, "length of %s is too long!\n", tokenized_input);
				}
				else
				{
					strcpy(input[i], tokenized_input);
				}
			}
			else
			{
				break;
			}
			tokenized_input = strtok(NULL, " ");
		}

		// Processing input array
		if (!strcmp(input[0], QUIT_CMD))
		{
			break;
		}
		else if (!strcmp(input[0], "nrf24l01_init"))
		{
			init_nrf24l01(fd);
		}
		else if (!strcmp(input[0], "nrf24l01_read_reg"))
		{
			read_register(fd, input[1]);
		}
		else
		{
			process_user_input(fd, input, categories, 0);
		}

		printf("\n");
	}

	printf("Exiting test program..\n");

	return 0;
}
