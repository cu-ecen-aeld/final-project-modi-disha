
//@Ref: https://github.com/ControlEverythingCommunity/ADXL345/blob/master/C/ADXL345.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include "i2c-dev.h"

void main() 
{
	// Create I2C bus
	int file;
	char *bus = (char*)"/dev/i2c-1";
	if ((file = open(bus, O_RDWR)) < 0) 
	{
		printf("Failed to open the bus. \n");
		exit(1);
	}
	// Get I2C device, ADXL345 I2C address is 0x53(83)
	ioctl(file, I2C_SLAVE, 0x53);

	// Select Bandwidth rate register(0x2C)
	// Normal mode, Output data rate = 100 Hz(0x0A)
	char config[2]={0};
	config[0] = 0x2C;
	config[1] = 0x0A;
	write(file, config, 2);
	// Select Power control register(0x2D)
	// Auto-sleep disable(0x08)
	config[0] = 0x2D;
	config[1] = 0x08;
	write(file, config, 2);
	// Select Data format register(0x31)
	// Self test disabled, 4-wire interface, Full resolution, range = +/-2g(0x08)
	config[0] = 0x31;
	config[1] = 0x08;
	write(file, config, 2);
	sleep(1);

	// Read 6 bytes of data from register(0x32)
	// xAccl lsb, xAccl msb, yAccl lsb, yAccl msb, zAccl lsb, zAccl msb
	char reg[1] = {0x32};
	write(file, reg, 1);
	char data[6] ={0};
	if(read(file, data, 6) != 6)
	{
		printf("Erorr : Input/output Erorr \n");
		exit(1);
	}
	else
	{
		// Convert the data to 10-bits
		int xAccl = ((data[1] & 0x03) * 256 + (data[0] & 0xFF));
		if(xAccl > 511)
		{
			xAccl -= 1024;
		}

		int yAccl = ((data[3] & 0x03) * 256 + (data[2] & 0xFF));
		if(yAccl > 511)
		{
			yAccl -= 1024;
		}

		int zAccl = ((data[5] & 0x03) * 256 + (data[4] & 0xFF));
		if(zAccl > 511)
		{
			zAccl -= 1024;
		}

		// Output data to screen
		printf("Acceleration in X-Axis : %d \n", xAccl);
		printf("Acceleration in Y-Axis : %d \n", yAccl);
		printf("Acceleration in Z-Axis : %d \n", zAccl);
	}
}
