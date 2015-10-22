#ifndef CMD_HELPER_FUNCTIONS_
#define CMD_HELPER_FUNCTIONS_

int send_cmd(int fd, const char* cmd_buffer, char *save_buffer, size_t cmd_count, size_t save_count)
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

int command_result(char return_code)
{
	if (!return_code)
	{
		fprintf(stdout, "Command executed successfully\n");
		return 0;
	}
	else
	{
		fprintf(stdout, "Command failed!\n");
		return -1;
	}
}

void print_as_binary(char c)
{
	fprintf(stdout, "%c%c%c%c %c%c%c%c",
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

int process_user_input(int fd, char user_input[MAX_ARGUMENT_CNT][MAX_ARGUMENT_LEN], const struct command cmd_list[], int current_index)
{
	char save_buffer[CMD_SIZE] = "";
	const struct command *cmd;
	for (cmd = cmd_list; cmd->name; ++cmd)
	{
		if (strcmp(user_input[current_index], cmd->name) == 0)
		{
			return cmd->sub_cmd ? process_user_input(fd, user_input, cmd->sub_cmd, current_index+1)
								: cmd->func(fd, user_input, sizeof(user_input), save_buffer, sizeof(save_buffer), current_index+1);
		}
	}
	printf("Invalid command! Type \"help\" for a list of commands\n");
	return -1;
}

#endif
