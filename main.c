#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define DEFAULT_DEVICE_PATH "/dev/mcp2210_usb2spi"

int main(int argc, char *argv[])
{
	int fd;
	char *dev = DEFAULT_DEVICE_PATH;

	printf("Starting test program..\n");

	printf("Opening %s\n", DEFAULT_DEVICE_PATH);
	fd = open(dev, O_RDWR);

	if (fd < 0)
	{
		printf("Could not open file, error: %i\n", errno);
	}

	printf("Exiting test program..\n");

	return 0;
}
