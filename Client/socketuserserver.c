/* SocketServer application for AESD final project. This file implements Socket Server code 
 * which accepts connection from the client.
 * Author: Disha Modi */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<linux/fs.h>
#include<fcntl.h>
#include<unistd.h>
#include<signal.h>
#include<libgen.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string.h>
#include<netdb.h>
#include<syslog.h>
#include<pthread.h>
#include<sys/queue.h>
#include<sys/time.h>
#include<time.h>
#include<stdbool.h>

#define PORT "9000"
#define BACKLOG 10

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
    syslog(LOG_DEBUG,"Caught SIGINT Signal exiting\n");
  if(sig == SIGTERM)
    syslog(LOG_DEBUG,"Caught SIGTERM Signal exiting\n"); 

  shutdown(newfd,SHUT_RDWR);
  shutdown(sockfd,SHUT_RDWR);
  _exit(0);
}

/// Thread parameter structure
typedef struct
{
    int threadIdx;
} threadParams_t;

/// Thread handler writes pseudo sensor 1 data to the socket
void* threadhandler1(void* thread_param)
{
	while(signal_flag)
	{	
		char rcv_cmd[80]={0}; 
     
	        //receive the string
	 	ssize_t length = recv(newfd, rcv_cmd, sizeof(rcv_cmd), 0);
	 	
		rcv_cmd[length] = '\0';
	    
	 	//check if read failed
	 	if(length == -1) 
	 	{
	 		perror("recv");
	 		printf("recv failed");
	 	}
	        
		printf("%s\n", rcv_cmd);
		
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

	pthread_t thread1; 

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
		
	inet_ntop(opp_addr.ss_family, get_in_addr((struct sockaddr *)&opp_addr),
                  s, sizeof s);
	
        threadParams[0].threadIdx = 1;
	
	/// create pthread and pass thread index
	if((pthread_create(&thread1, NULL, &threadhandler1, (void *)&(threadParams[0]))) != 0)
	{
		printf("thread1 creation failed\n");
	  syslog(LOG_ERR, "pthread create failed.");
	  return -1;
	}	
	
	 pthread_join(thread1, NULL);
 }
 
 printf("Signal received, shutting down...\n");
}

  
