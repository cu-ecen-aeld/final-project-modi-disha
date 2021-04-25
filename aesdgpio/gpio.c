/* gpio.c
* GPIO interface code for AESD Final Project. 
* Ref: https://elinux.org/RPi_GPIO_Code_Samples 
*
* This file blinks GPIO 4 (P1-07) while reading GPIO 24 (P1_18).
*/

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#define IN  0
#define OUT 1

#define LOW  0
#define HIGH 1

#define ECHO 2 /* P1-18 */
#define TRIG 3  /* P1-07 */

static int GPIOExport(int pin)
{
	char buffer[BUFFER_MAX];
	ssize_t bytes_written;
	int fd;

	fd = open("/sys/class/gpio/export", O_WRONLY);
	if (-1 == fd) {
		fprintf(stderr, "Failed to open export for writing!\n");
		return(-1);
	}

	bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
	write(fd, buffer, bytes_written);
	close(fd);
	return(0);
}

static int GPIOUnexport(int pin)
{
	char buffer[BUFFER_MAX];
	ssize_t bytes_written;
	int fd;

	fd = open("/sys/class/gpio/unexport", O_WRONLY);
	if (-1 == fd) {
		fprintf(stderr, "Failed to open unexport for writing!\n");
		return(-1);
	}

	bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
	write(fd, buffer, bytes_written);
	close(fd);
	return(0);
}

static int GPIODirection(int pin, int dir)
{
	static const char s_directions_str[]  = "in\0out";

	char path[DIRECTION_MAX];
	int fd;

	snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/direction", pin);
	fd = open(path, O_WRONLY);
	if (-1 == fd) {
		fprintf(stderr, "Failed to open gpio direction for writing!\n");
		return(-1);
	}

	if (-1 == write(fd, &s_directions_str[IN == dir ? 0 : 3], IN == dir ? 2 : 3)) {
		fprintf(stderr, "Failed to set direction!\n");
		return(-1);
	}

	close(fd);
	return(0);
}

static int GPIORead(int pin)
{

	char path[VALUE_MAX];
	char value_str[3];
	int fd;

	snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
	fd = open(path, O_RDONLY);
	if (-1 == fd) {
		fprintf(stderr, "Failed to open gpio value for reading!\n");
		return(-1);
	}

	if (-1 == read(fd, value_str, 3)) {
		fprintf(stderr, "Failed to read value!\n");
		return(-1);
	}

	close(fd);

	return(atoi(value_str));
}

static int GPIOWrite(int pin, int value)
{
	static const char s_values_str[] = "01";

	char path[VALUE_MAX];
	int fd;

	snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
	fd = open(path, O_WRONLY);
	if (-1 == fd) {
		fprintf(stderr, "Failed to open gpio value for writing!\n");
		return(-1);
	}

	if (1 != write(fd, &s_values_str[LOW == value ? 0 : 1], 1)) {
		fprintf(stderr, "Failed to write value!\n");
		return(-1);
	}

	close(fd);
	return(0);
}


double getTimeUsec(void) 
{ 
	struct timespec event_ts = {0, 0}; 
	clock_gettime(CLOCK_REALTIME, &event_ts); 
	return ((event_ts.tv_sec)* (int)1e6) + ((event_ts.tv_nsec)/1000.0); 
} 

/// main function
int main(int argc, char *argv[])
{

	double start;
	double stop;
	double diff;
	double distance;

	// Enable GPIO pins
	if (-1 == GPIOExport(TRIG) || -1 == GPIOExport(ECHO))
		return(1);

	// Set GPIO directions

	if (-1 == GPIODirection(TRIG, OUT) || -1 == GPIODirection(ECHO, IN))
		return(2);

	while(1)
	{
		// Write GPIO value
		if (-1 == GPIOWrite(TRIG, 1))
			return(3);
		usleep(30);
		if (-1 == GPIOWrite(TRIG, 0))
			return(3);

		// Read GPIO value

		start = getTimeUsec();
		printf(" start %f\n", start);
		while(GPIORead(ECHO)==1);
		stop = getTimeUsec();
		
		printf(" stop %f\n", stop);

		diff = stop -start;

		distance = diff/580.8;
		printf(" distance %f\n", distance);

		usleep(500*1000);
	}

	// Disable GPIO ECHOs
	if (-1 == GPIOUnexport(TRIG) || -1 == GPIOUnexport(ECHO))
		return(4);

	return(0);
}
