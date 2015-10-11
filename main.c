#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define DEFAULT_DEVICE_PATH "/dev/mcp2210_usb2spi"
#define CMD_SIZE 64
#define MAX_ARGUMENT_CNT 6
#define MAX_ARGUMENT_LEN 64

#define QUIT_CMD "q"
#define GET_GPIO_DIR_CMD "get_gpio_dir"
#define SET_GPIO_DIR_CMD "set_gpio_dir"
#define SET_GPIO_VAL_CMD "set_gpio_val"

int process_cmd(int fd, const char* cmd_buffer, char *save_buffer, size_t cmd_count, size_t save_count)
{
	int bytes_written = write(fd, cmd_buffer, cmd_count);
	if (bytes_written < 0)
	{	
		fprintf(stdout, "Error writing command: %i\n", errno);
		return errno;
	}
	int bytes_read = read(fd, save_buffer, save_count);
	if (bytes_read < 0)
	{
		fprintf(stdout, "Error reading from device: %i\n", errno);
		return errno;
	}
	return 0;
}

void print_as_binary(char c)
{
	fprintf(stdout, "%c%c%c%c %c%c%c%c ",
		(c & 0x80) ? '1' : '0',
		(c & 0x40) ? '1' : '0',
		(c & 0x20) ? '1' : '0',
		(c & 0x10) ? '1' : '0',
		(c & 0x08) ? '1' : '0',
		(c & 0x04) ? '1' : '0',
		(c & 0x02) ? '1' : '0',
		(c & 0x01) ? '1' : '0');
}

// Process a 1 byte hex input and return as a char
// If input is not in FF format, then return '\0'
char process_hex(const char* src)
{
	if (strlen(src) != 2)
	{
		return '\0';
	}
	unsigned int i;
	char retval = 0;
	for (i = 0; i < 2; ++i)
	{
		char intermediate = 0 + src[i] - '0';
		if (src[i] >= 'A' && src[i] <= 'F')
		{
			intermediate -= 7;
		}
		retval |= intermediate << 4 * (1 - i);
	}
	return retval;
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
		unsigned int i = 0;
		for (i; i < MAX_ARGUMENT_CNT; ++i)
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
		if (strcmp(input[0], QUIT_CMD) == 0)
		{
			break;
		}
		// Getting GPIO direction
		else if (strcmp(input[0], GET_GPIO_DIR_CMD) == 0)
		{
			cmd_buffer[0] = 0x33;
			if (process_cmd(fd, cmd_buffer, save_buffer, sizeof(cmd_buffer), sizeof(save_buffer)) != 0)
			{
				break;
			}
			char cmd_sucess = save_buffer[1];
			char gpio_dir[2];
			gpio_dir[0] = save_buffer[4];
			gpio_dir[1] = save_buffer[5];

			fprintf(stdout, "Results: ");
			print_as_binary(gpio_dir[1]);
			print_as_binary(gpio_dir[0]);
			fprintf(stdout, "\n");
		}
		else if (strcmp(input[0], SET_GPIO_DIR_CMD) == 0)
		{
			cmd_buffer[0] = 0x32;
			cmd_buffer[5] = process_hex(input[1]);
			cmd_buffer[4] = process_hex(input[2]);
			if (process_cmd(fd, cmd_buffer, save_buffer, sizeof(cmd_buffer), sizeof(save_buffer)) != 0)
			{
				break;
			}

			char cmd_success = save_buffer[1];
			if (cmd_success == 0)
			{
				fprintf(stdout, "Success!\n");
			}
			else
			{
				fprintf(stdout, "Failed!\n");
			}
		}
		else if (strcmp(input[0], SET_GPIO_VAL_CMD) == 0)
		{
			cmd_buffer[0] = 0x30;
			cmd_buffer[5] = process_hex(input[1]);
			cmd_buffer[4] = process_hex(input[2]);
			if (process_cmd(fd, cmd_buffer, save_buffer, sizeof(cmd_buffer), sizeof(save_buffer)) != 0)
			{
				break;
			}
			
			char cmd_success = save_buffer[1];
			if (cmd_success == 0)
			{
				char gpio_dir[2];
				gpio_dir[0] = save_buffer[4];
				gpio_dir[1] = save_buffer[5];
				fprintf(stdout, "Values: ");
				print_as_binary(gpio_dir[1]);
				print_as_binary(gpio_dir[0]);
				fprintf(stdout, "\n");
			}
			else
			{
				fprintf(stdout, "Command failed!\n");
			}
		}
		else
		{
			printf("Invalid command! Type \"help\" for a list of commands\n");
		}

		/*
		while(tokenized_input)
		{
			if (strcmp(tokenized_input, "w") == 0)
			{
				printf("Attempting write..\n");
				int bytes_written;

				if (bytes_written != strlen("asd"))
				{
					printf("Could not write all bytes: %i\n", errno);
				}
				printf("Write complete\n");
			}
			tokenized_input = strtok(NULL, " ");
		}*/

		printf("\n");
	}
/*
	printf("Attempting to read..\n");
	int bytes_read = read(fd, &my_buffer, 64);
	
	if (bytes_read == -1)
	{
		printf("Could not read all bytes: %i\n", errno);
		return -1;
	}

	printf("Buffer: %s\n", my_buffer);
*/
	printf("Exiting test program..\n");

	return 0;
}
