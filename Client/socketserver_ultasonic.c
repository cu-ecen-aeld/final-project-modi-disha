/* SocketServer application for AESD final project. This file implements Socket Server code 
 * which accepts connection from the client.
 * Author: Disha Modi */
/* @Ref doc for I2C: https://github.com/ControlEverythingCommunity/ADXL345/blob/master/C/ADXL345.c */

/** Include libraries **/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <linux/fs.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <netdb.h>
#include <syslog.h>
#include <pthread.h>
#include <sys/queue.h>
#include <sys/time.h>
#include <time.h>
#include <stdbool.h>
#include <time.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

// Socket communication port
#define PORT "9000"
#define BACKLOG 10

// GPIO Pins and Directions
#define IN  0
#define OUT 1
#define LOW  0
#define HIGH 1

#define ECHO 20 /* GPIO 20 on RPi 3 */
#define TRIG 21  /* GPIO 21 on RPi 3 */

// Globals for socket programming
int sockfd, newfd;
char s[INET6_ADDRSTRLEN];
int signal_flag =1;
pthread_mutex_t socklock;
char rdBuff[80] = {'\0'};

//Signal HAndler function
void handle_sig(int sig)
{
  signal_flag=0;

  if(sig == SIGINT)
    syslog(LOG_DEBUG, "Caught SIGINT Signal exiting\n");
  if(sig == SIGTERM)
    syslog(LOG_DEBUG, "Caught SIGTERM Signal exiting\n"); 

  shutdown(newfd,SHUT_RDWR);
  shutdown(sockfd,SHUT_RDWR);

  _exit(0);
}

/// Thread parameter structure
typedef struct
{
    int threadIdx;
} threadParams_t;

/***********************************************************/
// I2C CODE //
void i2c_sensor_read(int *xAccl, int *yAccl, int *zAccl) 
{
	// Create I2C bus
	int i2c_file;
	char *bus = (char*)"/dev/i2c-1"; // i2c-1
        int bytes = 0;

	// Open i2c-1 for read-write
	if(( i2c_file = open(bus, O_RDWR)) < 0 ) 
	{
		printf("Failed to open the bus.\n");
		exit(1);
	}

	// ADXL345 I2C sensor device address is 0x53(83)
	if( ioctl(i2c_file, I2C_SLAVE, 0x53) < 0 )
	{
		printf("ioctl 0x53 failed.\n");
		exit(1);
	}

	/// Select Bandwidth rate register(0x2C)
	/// Normal mode, Output data rate = 100 Hz(0x0A)
	char config[2]={0};
	config[0] = 0x2C;
	config[1] = 0x0A;

	if(( bytes = write(i2c_file, &config[0], 2)) < 0 )
	{
		printf("write word 0x0a2c failed with %d error %s. \n", bytes, strerror(errno));
		exit(1);
	}

	// Select Power control register(0x2D)
	// Auto-sleep disable(0x08)
	config[0] = 0x2D;
	config[1] = 0x08;

	if(( bytes = write(i2c_file, &config[0], 2)) < 0 )
	{
		printf("write word 0x082d failed with %d error %s. \n", bytes, strerror(errno));
		exit(1);
	}

	// Select Data format register(0x31)
	// Self test disabled, 4-wire interface, Full resolution, range = +/-2g(0x08)
	config[0] = 0x31;
	config[1] = 0x08;

	if(( bytes = write(i2c_file, &config[0], 2)) < 0 )
	{
		printf("write word 0x0831 failed with %d error %s. \n", bytes, strerror(errno));
		exit(1);
	}

	/// Sensor processing time
	sleep(1);

	// Read 6 bytes of data from register(0x32)
	// xAccl lsb, xAccl msb, yAccl lsb, yAccl msb, zAccl lsb, zAccl msb
	char reg[1] = {0x32};

	if(( bytes = write(i2c_file, &reg[0], 1)) < 0 )
	{
		printf("write word 0x0831 failed %d bytes %s. \n", bytes, strerror(errno));
		exit(1);
	} 

	char data[6] ={0};

	if(( bytes = read(i2c_file, &data[0], 6)) != 6 )
	{
		printf("Erorr : Input/output read Erorr %s \n", strerror(errno));
		exit(1);
	}
	
	// Convert the data to 10-bits
	*xAccl = (( data[1] & 0x03) * 256 + (data[0] & 0xFF ));

	if( *xAccl > 511 )
	{
		*xAccl -= 1024;
	}

	*yAccl = (( data[3] & 0x03) * 256 + (data[2] & 0xFF ));

	if( *yAccl > 511 )
	{
		*yAccl -= 1024;
	}

	*zAccl = (( data[5] & 0x03) * 256 + (data[4] & 0xFF ));

	if( *zAccl > 511 )
	{
		*zAccl -= 1024;
	}

	// Output data to screen
	printf("Acceleration in X-Axis : %d \n", *xAccl);
	printf("Acceleration in Y-Axis : %d \n", *yAccl);
	printf("Acceleration in Z-Axis : %d \n", *zAccl);
	
}
/*****************************************************************/


/***********************************************************/
// GPIO CODE //

static int GPIOExport(int pin)
{
	#define BUFFER_MAX 3
	char buffer[BUFFER_MAX];
	ssize_t bytes_written;
	int fd;

	fd = open("/sys/class/gpio/export", O_WRONLY);
	if (-1 == fd) 
	{
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

	#define DIRECTION_MAX 35
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
	#define VALUE_MAX 30
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

/// getTimeUsec function to get current time
double getTimeUsec(void) 
{ 
	struct timespec event_ts = {0, 0}; 
	clock_gettime(CLOCK_REALTIME, &event_ts); 
	return ((event_ts.tv_sec)* (int)1000000) + ((event_ts.tv_nsec)/1000.0); 
} 


/// GPIO_init function to initialize GPIO pins
int GPIO_init()
{
	/* Enable GPIO pins*/
	if (-1 == GPIOExport(TRIG) || -1 == GPIOExport(ECHO))
		return(1);

	/* Set GPIO directions */
	if (-1 == GPIODirection(TRIG, OUT) || -1 == GPIODirection(ECHO, IN))
		return(2);
}


/// read_sensor_data function reads GPIO pin output 
int read_sensor_data()
{
	double start;
	double stop;
	double diff;
	double distance;

	if (-1 == GPIOWrite(TRIG, 1))
		return(3);

	usleep(30);

	if (-1 == GPIOWrite(TRIG, 0))
	 	return(3);

	// Read GPIO value
	/* For ultrasonic sensor calculating the time 
	   ECHO pin is staying high */
	start = getTimeUsec();
	printf(" start %f\n", start);

	while(GPIORead(ECHO)==1);

	stop = getTimeUsec();	
	printf(" stop %f\n", stop);

	diff = stop - start;

	distance = (diff/58) * 100;
	printf(" distance %d\n", distance);

	return distance;
}

/***********************************************************************/


/// Thread handler writes GPIO sensor 1 data to the socket
void* threadhandler1(void* thread_param)
{
   	GPIO_init();
	
	/// Execute thread until sigint or sigterm
	while(signal_flag)
	{	
		int k=0, rc=0;

		time_t r_time;
		struct tm *timeinfo;
		char buf[30];
		time(&r_time);

		/// read sensor data
		k = read_sensor_data();
		
		timeinfo = localtime(&r_time);
		strftime(buf, 80,"%x-%H:%M %p ", timeinfo);
		
	        pthread_mutex_lock(&socklock); /// protect rdBuff as it is shared  	
		if(k < 18)
		{
			sprintf(rdBuff, "%s !Alert S3: %d\n",buf, k);
		}
		else
		{
			sprintf(rdBuff, "%s sensor 3: %d\n",buf, k);
		}
		
		/// send the data over socket
		rc = send(newfd, rdBuff, strlen(rdBuff), MSG_DONTWAIT);

	        pthread_mutex_unlock(&socklock);

		if( rc < 0){
		  perror("Couldnt send sensor results to file\n");
		}
	
		sleep(1);
	}
	pthread_exit((void *)0);
}


/// Thread handler writes I2C sensor data to the socket
void* threadhandler2(void* thread_param)
{
	/// Execute thread until sigint or sigterm
	while(signal_flag)
	{
		/// LOG MSG TO SYSLOG: "Accepted connection from XXX"
		syslog(LOG_INFO, "Accepted connection from %s\n", s);
		
		int k=0,rc=0;
		
		time_t r_time;
		struct tm *timeinfo;
		char buf[30];
		time(&r_time);

		int xa, xb, xc;
		i2c_sensor_read(&xa, &xb, &xc); 
		
		timeinfo = localtime(&r_time);
		strftime(buf, 80,"%x-%H:%M %p ", timeinfo);
		
        	pthread_mutex_lock(&socklock);	

		if((xa < 0) && (xb < 0) && (xc < 0))
		{
			sprintf(rdBuff, "%s !Alert S4: x:%d y:%d z:%d\n",buf, xa, xb, xc);
		}
		else
		{
			sprintf(rdBuff, "%s sensor 4: %d\n",buf, k);
		}

		rc = send(newfd, rdBuff, strlen(rdBuff), MSG_DONTWAIT);
        	pthread_mutex_unlock(&socklock);
		if( rc < 0){
		  perror("Couldnt send sensor results to file\n");
		}
	
		sleep(1);
	}

	pthread_exit((void *)0);
}

// This routine returns client socket address
//ref:https://beej.us/guide/bgnet/html/#cb20-1
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {                   
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);   // for IPv6
}

/// Main function performs socket binding and calls thread handlers.
// This function executes code until sigint or sigterm signal.
int main(int argc, char *argv[])
{
	
  threadParams_t threadParams[2];
  signal(SIGTERM,handle_sig);
  signal(SIGINT,handle_sig);

  if(pthread_mutex_init(&socklock, NULL) != 0)
  {
	  printf("mutex init failed.\n");
	  return -1; 
  }
	  
  struct sockaddr_storage opp_addr;
  struct addrinfo ref, *res, *p;
  memset(&ref, 0, sizeof(ref));
  ref.ai_family = AF_UNSPEC;
  ref.ai_socktype = SOCK_STREAM;
  ref.ai_flags = AI_PASSIVE;

  if(getaddrinfo(NULL, PORT, &ref, &res) != 0)
  {
    syslog(LOG_ERR, "getaddrinfo failed.");
    return -1;
  }

	  /// socket and bind
	for(p = res; p != NULL; p = p->ai_next)
	{
	
	  if((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0)
	  {
			perror("Socket failed");
	  }
	
	  if(bind(sockfd, res->ai_addr, res->ai_addrlen) < 0)
	  {
			perror("bind failed");
	  }
	
	  break;
	}
	
	if(p == NULL)
	{
	  syslog(LOG_ERR, "Socket bind failed.");
		return -1;
	}
	
	freeaddrinfo(res);

	/// socket listen
	if(listen(sockfd, BACKLOG) < 0)
	{
	  syslog(LOG_ERR, "Socket listen failed.");
	  return -1;
	}

	pthread_t thread1, thread2; 

        while(signal_flag)
	{

	socklen_t addr_size = sizeof(opp_addr);
		
	/// accept socket connection
	newfd = accept(sockfd, (struct sockaddr *)&opp_addr, &addr_size);
	if(newfd < 0)
	{
		syslog(LOG_ERR, "Socket accept failed.");
		if(!signal_flag) 
		{
			break;
		}
                exit (EXIT_FAILURE);
	}
		
	inet_ntop(opp_addr.ss_family, get_in_addr((struct sockaddr *)&opp_addr), s, sizeof s);
	
        threadParams[0].threadIdx = 1;
	threadParams[1].threadIdx = 2;
	
	/// create pthread and pass thread index
	if((pthread_create(&thread1, NULL, &threadhandler1, (void *)&(threadParams[0]))) != 0)
	{
		printf("thread1 creation failed\n");
		syslog(LOG_ERR, "pthread create failed.");
	        return -1;
	}
	
	/// create pthread and pass socket id, and global mutex in thread arguments
	if((pthread_create(&thread2, NULL, &threadhandler2, (void *)&(threadParams[1]))) != 0)
	{
		printf("thread2 creation failed\n");
	        syslog(LOG_ERR, "pthread create failed.");
	        return -1;
	}	
	
	 pthread_join(thread1, NULL);
	 pthread_join(thread2, NULL);	
 }
 
 printf("Signal received, shutting down...\n");
}

  
