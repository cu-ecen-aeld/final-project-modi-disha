/* I2C REFERENCE https://raspberry-projects.com/pi/programming-in-c/i2c/using-the-i2c-interface */
/* Magnetometer REFERENCE https://www.enib.fr/~kerhoas/rescapt_mpu9250.html */
#include <unistd.h>				//Needed for I2C port
#include <fcntl.h>				//Needed for I2C port
#include <stdint.h>
#include <sys/ioctl.h>			//Needed for I2C port
//#include <linux/i2c-dev.h>		//Needed for I2C port
#include "i2c-dev.h"
#include "MadgwickAHRS.h"
int main()
{
	int file_i2c, file_i2c_mag;
	//int length;
	uint32_t buffer[18] = {0};
	uint32_t ax =0, ay = 0, az = 0, gx = 0, gy = 0, gz = 0, mx = 0, my = 0, mz = 0;

	//----- OPEN THE I2C BUS -----
	char *filename = (char*)"/dev/i2c-1";
	if ((file_i2c = open(filename, O_RDWR)) < 0)
	{
		//ERROR HANDLING: you can check errno to see what went wrong
		printf("Failed to open the i2c bus");
		return -1;
	}
	if ((file_i2c_mag = open(filename, O_RDWR)) < 0)
	{
		//ERROR HANDLING: you can check errno to see what went wrong
		printf("Failed to open magnetometer  the i2c bus");
		return -1;
	}

	int addr = 0x68;          //<<<<<The I2C address of the slave
	int addr_mag = 0x0C; 
	if (ioctl(file_i2c, I2C_SLAVE, addr) < 0)
	{
		printf("Failed to acquire bus access and/or talk to slave.\n");
		//ERROR HANDLING; you can check errno to see what went wrong
		return -1 ;
	}
	if (ioctl(file_i2c_mag, I2C_SLAVE, addr_mag) < 0)
	{
		printf("Failed to acquire bus access for magnetometer and/or talk to slave.\n");
		//ERROR HANDLING; you can check errno to see what went wrong
		return -1 ;
	}
	i2c_smbus_write_byte_data(file_i2c, 0x37, 0x22);
	/* New code */
	while(1)
	{
	
		/* Reading accelerometer values */
		buffer[0] = i2c_smbus_read_byte_data(file_i2c, 0x3B); // AccelerometerX High Byte
		buffer[1] = i2c_smbus_read_byte_data(file_i2c, 0x3C); // AccelerometerX Low Byte
		buffer[2] = i2c_smbus_read_byte_data(file_i2c, 0x3D); // AccelerometerY High Byte
		buffer[3] = i2c_smbus_read_byte_data(file_i2c, 0x3E); // AccelerometerY Low Byte
		buffer[4] = i2c_smbus_read_byte_data(file_i2c, 0x3F); // AccelerometerZ High Byte
		buffer[5] = i2c_smbus_read_byte_data(file_i2c, 0x40); // AccelerometerZ Low Byte

	//	 printf("Accelerometer values are %d, %d, %d, %d, %d, %d\n", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);

		/* Reading gyroscope values */
		buffer[6] = i2c_smbus_read_byte_data(file_i2c, 0x43); // GyroscopeX High Byte
		buffer[7] = i2c_smbus_read_byte_data(file_i2c, 0x44); // GyroscopeX Low Byte
		buffer[8] = i2c_smbus_read_byte_data(file_i2c, 0x45); // GyroscopeY High Byte
		buffer[9] = i2c_smbus_read_byte_data(file_i2c, 0x46); // GyroscopeY Low Byte
		buffer[10] = i2c_smbus_read_byte_data(file_i2c, 0x47); // GyroscopeZ High Byte
		buffer[11] = i2c_smbus_read_byte_data(file_i2c, 0x48); // GyroscopeZ Low Byte
		
	//	 printf("Gyroscope  values are %d, %d, %d, %d, %d, %d\n", buffer[6], buffer[7], buffer[8], buffer[9], buffer[10], buffer[11]);
		i2c_smbus_write_byte_data(file_i2c_mag, 0x0A, (1 << 4) | 0x06);		
		/* Reading Magnetometer values */
		uint32_t magStatus1 = 0;
		uint32_t deviceID = i2c_smbus_read_byte_data( file_i2c_mag,0x00);
		// printf("Device ID is %x\n",deviceID);
		do
  		{
     			magStatus1 = i2c_smbus_read_byte_data( file_i2c_mag,0x02);
 		}while (!(magStatus1&0x0001));

		buffer[12] = i2c_smbus_read_byte_data(file_i2c_mag, 0x04); // MagnetometerX High Byte
		buffer[13] = i2c_smbus_read_byte_data(file_i2c_mag, 0x03); // MagnetometerX Low Byte
		buffer[14] = i2c_smbus_read_byte_data(file_i2c_mag, 0x06); // MagnetometerY High Byte
		buffer[15] = i2c_smbus_read_byte_data(file_i2c_mag, 0x05); // MagnetometerY Low Byte
		buffer[16] = i2c_smbus_read_byte_data(file_i2c_mag, 0x08); // MagnetometerZ High Byte
		buffer[17] = i2c_smbus_read_byte_data(file_i2c_mag, 0x07); // MagnetometerZ Low Byte
		
		// printf("Magnetometer values are %d, %d, %d, %d, %d, %d\n", buffer[12], buffer[13], buffer[14], buffer[15], buffer[16], buffer[17]);
		/* Converting the accelerometer values into 16 bits */
		ax = buffer[0] << 8 | buffer[1];
		ay = buffer[2] << 8 | buffer[3];
		az = buffer[4] << 8 | buffer[5];

		/* Converting the gyroscope values into 16 bits */
		gx = buffer[6] << 8 | buffer[7];
		gy = buffer[8] << 8 | buffer[9];
		gz = buffer[10] << 8 | buffer[11];

		/* Converting the gyroscope values into 16 bits */
		mx = buffer[12] << 8 | buffer[13];
		my = buffer[14] << 8 | buffer[15];
		mz = buffer[16] << 8 | buffer[17];
		
		//printf("The magnetometer values are %d, %d, %d\n", mx, my,mz);
		float roll, pitch, yaw;
		MadgwickAHRSupdate((float)ax, (float)ay, (float)az, (float)gx, (float)gy, (float)gz, (float)mx, (float)my, (float)mz,&roll,&pitch, &yaw);	
		// int k = 0;
		// for(k = 0; k < 90000000; k++);
	}
}
