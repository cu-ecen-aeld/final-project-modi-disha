#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include<stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include <arpa/inet.h>
#include <errno.h>
#include<syslog.h>
#include <signal.h>
#include <stdint.h>

#define MAXRECVSTRING 30
#define PORTNO 9000
#define PORT "9000"
#define PORTNO1 9001
#define PORT1 "9001"
#define pFILE "/var/tmp/sensordata.txt"
#define pFILE1 "/var/tmp/sensordata1.txt"

int operation_switch =1;

//TCP socket descriptor
int socket_client, socket_client1;

//Signal HAndler function
void handle_sig(int sig)
{
  operation_switch=0;
  if(sig == SIGINT)
    syslog(LOG_DEBUG,"Caught SIGINT Signal exiting\n");
  if(sig == SIGTERM)
    syslog(LOG_DEBUG,"Caught SIGTERM Signal exiting\n");  
  shutdown(socket_client,SHUT_RDWR);
  _exit(0);
}
  
int main(int argc, char *argv[])
{
  signal(SIGTERM,handle_sig);
  signal(SIGINT,handle_sig);


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

  if((socket_client = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0)
  {
		perror("socket failed");
  }

  if(connect(socket_client, res->ai_addr, res->ai_addrlen) == -1)
  {
    printf("Connect to server failed\n");
  }
  
  if(getaddrinfo(NULL, PORT1, &ref, &res) != 0)
  {
    syslog(LOG_ERR, "getaddrinfo failed.");
    return -1;
  } 

  if((socket_client1 = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0)
  {
		perror("socket1 failed");
  }

  if(connect(socket_client1, res->ai_addr, res->ai_addrlen) == -1)
  {
    printf("Connect to server1 failed\n");
  }

  while(operation_switch)
  {
    char rcv_cmd[20]={0}; 
    char rcv_cmd1[20]={0}; 
    
    //receive the string
	ssize_t length = recv(socket_client, rcv_cmd, sizeof(rcv_cmd), 0);
	
    //receive the string
	ssize_t length1 = recv(socket_client1, rcv_cmd1, sizeof(rcv_cmd1), 0);
	
	rcv_cmd[length] = '\0';
	//check if read failed
	if(length == -1) 
	{
		perror("recv");
		printf("recv failed");
	}
	
	rcv_cmd1[length] = '\0';
	//check if read failed
	if(length1 == -1) 
	{
		perror("recv1");
		printf("recv1 failed");
	}
    
	//open file for write
	int fd = open(pFILE, O_RDONLY | O_WRONLY | O_CREAT | O_APPEND, 0644); 
	if(fd < 0)
	{
		printf("File open failed");
	}

	ssize_t rc = write(fd, (void *)rcv_cmd, strlen(rcv_cmd));
    
    if( rc < 0){
      perror("Couldnt write sensor results to file\n");
    }
    
	ssize_t rc1 = write(fd, (void *)rcv_cmd1, strlen(rcv_cmd1));
    
    if( rc1 < 0){
      perror("Couldnt write sensor1 results to file\n");
    }
    
    close(fd);

 }
}
  //shutdown(socket_client,SHUT_RDWR);
  