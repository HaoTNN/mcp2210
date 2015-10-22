#ifndef _NRF24L01_H
#define _NRF24L01_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "main.h"
#include "cmd_helper_functions.c"

/* Using the NRF24L01:
 *
 * To send commands to the NRF24L01:
 * Commands must be started by a high to low transition on CSN
 * E.g, the process of sending a read command is as follows:
 * 
 * LOW -> CSN
 * Wait for at least 10us
 * Sendbyte value of 0x00 (Read register at 00000)
 * Wait for at least 10us
 * Sendbyte value of 0xFF (NOP dummy byte)
 * Wait for at least 10us
 * HIGH -> CSN
 *
 * Commands and Register maps can be found on the NRF24L01 Datasheet online
 */

/* Settings we're using:
 *
 * SPI Settings:
 * CSN: GP2 as CS				- Chip Select pin
 * CE: GP5 cs GPIO				- Chip Enable pin, this will control whether NRF24L01 is in Rx or Tx mode
 * Active CS Value: LOW			- CS value when SPI transfer is active, CS must transition from HIGH to LOW to perform a command
 * Idle CS Value: HIGH			- CS value when there is no SPI transfer
 * Bitrate: 9600				- Transfer rate, no real reason why 9600 is chosen
 * CS to Data delay: 100us		- Delay between CS value change and first byte of data, NRF24L01 requires 10us, but MCP2210 requires quanta of 100us
 * Data to Data delay: 100us	- Delay between data bytes, see above on why 100us
 * Last Data Byte to CS: 100us	- Delay between last data byte and CS value change, see above on why 100us
 *
 * GPIO Settings:
 * GP2: CS (0x01)
 * GP5: GPIO (0x00)
 *
 */
void init_nrf24l01(int fd)
{
	char input[MAX_ARGUMENT_CNT][MAX_ARGUMENT_LEN];

	// What command to use?
	// Cmd to set up SPI settings
	strcpy(input[0], "ram");
	strcpy(input[1], "spi");
	strcpy(input[2], "set");

	// Arguments to command
	// Bitrate, 0x00002580
	strcpy(input[3], "80");
	strcpy(input[4], "25");
	strcpy(input[5], "00");
	strcpy(input[6], "00");

	// Idle CS value, 0x0004
	strcpy(input[7], "04");
	strcpy(input[8], "00");

	// Active CS value, 0x0000
	strcpy(input[9], "00");
	strcpy(input[10], "00");

	// CS to data delay, 0x0001
	strcpy(input[11], "01");
	strcpy(input[12], "00");

	// Last byte to CS delay, 0x0001
	strcpy(input[13], "01");
	strcpy(input[14], "00");

	// Data to Data delay, 0x0001
	strcpy(input[15], "01");
	strcpy(input[16], "00");

	// Number of bytes to transfer, 2 bytes for now - 0x0002
	strcpy(input[17], "02");
	strcpy(input[18], "00");

	// SPI Mode - 0x00
	strcpy(input[19], "00");

	// Send it off
	fprintf(stdout, "Executing %s %s %s\n", input[0], input[1], input[2]);
	if (process_user_input(fd, input, categories, 0))
	{
		fprintf(stdout, "Init failed!\n");
		return;
	}

	// Clean up array and input cmd to set up GPIO settings
	memset(input, 0, sizeof(input[0][0]) * MAX_ARGUMENT_CNT * MAX_ARGUMENT_LEN);
	strcpy(input[0], "ram");
	strcpy(input[1], "gpio_chip");
	strcpy(input[2], "set");

	// GP pin designation
	strcpy(input[3], "00");
	strcpy(input[4], "00");
	strcpy(input[5], "01");
	strcpy(input[6], "00");
	strcpy(input[7], "00");
	strcpy(input[8], "00");
	strcpy(input[9], "00");
	strcpy(input[10], "00");
	strcpy(input[11], "00");

	// GP Default value, 0x01FF
	strcpy(input[12], "FF");
	strcpy(input[13], "01");

	// GP Default direction, 0x0000
	strcpy(input[14], "00");
	strcpy(input[15], "00");

	// Other settings, 0x12 is default
	strcpy(input[16], "12");

	// Send it off
	fprintf(stdout, "Executing %s %s %s\n", input[0], input[1], input[2]);
	if (process_user_input(fd, input, categories, 0))
	{
		fprintf(stdout, "Init failed!\n");
		return;
	}
}

void read_register(int fd, const char *reg)
{
	char input[MAX_ARGUMENT_CNT][MAX_ARGUMENT_LEN];

	// Command syntax
	strcpy(input[0], "spi");
	strcpy(input[1], "transfer_data");

	// Arguments
	strcpy(input[2], "02");
	strcpy(input[3], "00");

	// Send command to read register
	char test = 0 + process_hex(reg);
	print_as_binary(test);

	// Get data back

	// print it out
}

#endif
