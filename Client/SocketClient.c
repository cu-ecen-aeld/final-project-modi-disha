/* SocketClient application for AESD final project. This file implements Socket Client code 
 * which connects to the servers specified into the arguments.
 * Author: Disha Modi */
// Ref for mqueue: https://www.softprayog.in/programming/interprocess-communication-using-posix-message-queues-in-linux
/// Include Libraries ///
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
#include<mqueue.h>

#define PORT "9000"
#define pFILE "/var/tmp/sensordata.txt"

#define SERVER_QUEUE_NAME   "/aesd-server"
#define QUEUE_PERMISSIONS 0660
#define MAX_MESSAGES 25
#define MAX_MSG_SIZE 256
#define MSG_BUFFER_SIZE MAX_MSG_SIZE + 10

uint8_t thread_count = 0;
int signal_flag = 1;
pthread_mutex_t socklock;

mqd_t aesdqueue;

// socket descriptor
int socket_client;

// thread parameter structure
typedef struct
{
    int threadIdx;
    int thread_socket;
    
} threadParams_t;


//Signal Handler function
void handle_sig(int sig)
{
  signal_flag = 0;
  if(sig == SIGINT)
    syslog(LOG_DEBUG,"Caught SIGINT Signal exiting\n");
  if(sig == SIGTERM)
    syslog(LOG_DEBUG,"Caught SIGTERM Signal exiting\n");  
  shutdown(socket_client,SHUT_RDWR);
  _exit(0);
}

/// Thread handler to receive data from servers and write it back to the file.
void* threadhandler(void* thread_param)
{
  threadParams_t *args = (threadParams_t *)thread_param;
  uint8_t thread_id = args->threadIdx;
  
  while(signal_flag)
  {
 	  
    char rcv_cmd[80]={0}; 
     
     //receive the string
 	ssize_t length = recv(args->thread_socket, rcv_cmd, sizeof(rcv_cmd), 0);
 	
        rcv_cmd[length] = '\0';
    
 	//check if read failed
 	if(length == -1) 
 	{
 		perror("recv");
 		printf("recv failed");
 	}
     
 	//open file for write
 	int fd = open(pFILE, O_RDONLY | O_WRONLY | O_CREAT | O_APPEND, 0644); 
 	if(fd < 0)
 	{
 		printf("File open failed");
 	}

 	pthread_mutex_lock(&socklock);
 	ssize_t rc = write(fd, (void *)rcv_cmd, strlen(rcv_cmd));
        pthread_mutex_unlock(&socklock);
    
        if( rc < 0)
        {
           perror("Couldnt write sensor results to file\n");
        }
        else
        {
	   bool alert_flag = 0;
	   int i = 0;
	   for(i=0; i<strlen(rcv_cmd); i++)
	   {
		if(rcv_cmd[i] == '!')
		{
			alert_flag = 1;
			break;
		}
	   }
		
    	 if(alert_flag)
    	 {
    		 //send(socket_client, rcv_cmd, strlen(rcv_cmd), MSG_DONTWAIT);
		 if (mq_send (aesdqueue, rcv_cmd, strlen (rcv_cmd) + 1, 0) == -1) 	             
                {
		    perror ("Client: Not able to send message to server");
		}
		
    	 }
	 alert_flag = 0;
        }
 	
     close(fd);
  }
  pthread_exit((void *)0);
}
  
/// Main function implements socket connect and calls threads. 
// Executes code until sigint or sigterm signal.
int main(int argc, char *argv[])
{
  thread_count = atoi(argv[1]);
  pthread_t thread[thread_count]; 
  threadParams_t threadParams[thread_count];
  signal(SIGTERM,handle_sig);
  signal(SIGINT,handle_sig);
  //uint8_t user_threadid = atoi(argv[1])+1;
  
  if(pthread_mutex_init(&socklock, NULL) != 0)
  {
	  printf("mutex init failed.\n");
	  return -1; 
  }

   struct mq_attr attr;

    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;

    if ((aesdqueue = mq_open (SERVER_QUEUE_NAME, O_WRONLY)) == -1) {
        perror ("Client: mq_open (server)");
        exit (1);
    }

  struct addrinfo ref, *res;
  memset(&ref, 0, sizeof(ref));
  ref.ai_family = AF_UNSPEC;
  ref.ai_socktype = SOCK_STREAM;
  
  printf("Client is trying to connect with %d servers.\n", atoi(argv[1]));

  while(signal_flag)
  {
	  
	for(uint8_t i=0; i<thread_count; i++)
	{
		threadParams[i].threadIdx = i+1;
		 if(getaddrinfo(argv[threadParams[i].threadIdx+1], PORT, &ref, &res) != 0)
		  {
		    syslog(LOG_ERR, "getaddrinfo failed.");
		    return -1;
		  }

		  if((threadParams[i].thread_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0)
		  {
				perror("socket failed");
		  }

		  if(connect(threadParams[i].thread_socket, res->ai_addr, res->ai_addrlen) == -1)
		  {
		    	printf("Connect to server failed\n");
		  }
		  else
		  {
			  printf("Client connected with Server %s successfully.", argv[threadParams[i].threadIdx+1]);
		  }
		
		/// create pthread and pass thread index
		if((pthread_create(&thread[i], NULL, &threadhandler, (void *)&(threadParams[i]))) != 0)
		{
		  printf("thread%d creation failed\n", i);
		  syslog(LOG_ERR, "pthread%d create failed.", i);
		  return -1;
		}
	}
	
	for(uint8_t i=0; i<thread_count; i++)
	{
		 pthread_join(thread[i], NULL);
	}

 }
  printf("Signal received, shutting down...\n");
}
 
  
