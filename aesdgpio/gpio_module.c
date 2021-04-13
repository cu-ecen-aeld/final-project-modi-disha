#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#define NUM_GPIO_PINS 21
#define MAX_GPIO_NUMBER 32
#define BUF_SIZE 4
#define PATH_SIZE 20

int main(int argc, char **argv)
{
	int i = 0, index = 0, value;
	int fd[NUM_GPIO_PINS];
	char path[PATH_SIZE];
	char buf[BUF_SIZE];
	char readBuf[2];
	int itr = 50;

	i = 4;
	snprintf(path, sizeof(path), "/dev/raspiGpio%d", i);
	fd[index] = open(path, O_WRONLY);
	if (fd[index] < 0) {
		perror("Error opening GPIO pin");
		exit(EXIT_FAILURE);
	}

	printf("Set GPIO pins to output, logic level :%d\n", 1);
	strncpy(buf, "out", 3);
	buf[3] = '\0';

	if (write(fd[index], buf, sizeof(buf)) < 0) {
		perror("write, set pin output");
		exit(EXIT_FAILURE);
	}

	do {

		strncpy(buf, "1", 1);
		buf[1] = '\0';

		if (write(fd[index], buf, sizeof(buf)) < 0) {
			perror("write, set GPIO state of GPIO pins");
			exit(EXIT_FAILURE);
		}

		strncpy(buf, "1", 0);
		buf[1] = '\0';

		if (write(fd[index], buf, sizeof(buf)) < 0) {
			perror("write, set GPIO state of GPIO pins");
			exit(EXIT_FAILURE);
		}

		usleep(500 * 1000);
	}
	while(itr--);



	index++;
	i = 24;

	snprintf(path, sizeof(path), "/dev/raspiGpio%d", i);
	fd[index] = open(path, O_RDWR);
	if (fd[index] < 0) {
		perror("Error opening GPIO pin");
		exit(EXIT_FAILURE);
	}

	printf("Set GPIO pins to output, logic level :%d\n", 1);
	strncpy(buf, "in", 2);
	buf[2] = '\0';

	if (write(fd[index], buf, sizeof(buf)) < 0) {
		perror("write, set pin output");
		exit(EXIT_FAILURE);
	}

	if (read(fd[index], readBuf, 1) < 1) {
		perror("write, set pin input");
		exit(EXIT_FAILURE);
	}
	readBuf[1] = '\0';
	printf("GPIO pin: %d Logic level: %s\n", i, readBuf);



}
