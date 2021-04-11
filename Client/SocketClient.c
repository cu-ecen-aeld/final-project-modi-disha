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

#define MAXRECVSTRING 30
#define PORTNO 9000
#define PORT "9000"
#define pFILE "/var/tmp/sensordata.txt"

uint8_t thread_count = 0;
int signal_flag = 1;
pthread_mutex_t socklock;
//socket descriptor
int socket_client;

typedef struct
{
    int threadIdx;
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

void* threadhandler(void* thread_param)
{
  threadParams_t *args = (threadParams_t *)thread_param;
  uint8_t thread_id = args->threadIdx;
  
  int thread_socket;
	
  struct addrinfo ref, *res;
  memset(&ref, 0, sizeof(ref));
  ref.ai_family = AF_UNSPEC;
  ref.ai_socktype = SOCK_STREAM;
	
  if(getaddrinfo(argv[thread_id + 1], PORT, &ref, &res) != 0)
  {
	syslog(LOG_ERR, "getaddrinfo failed.");
	return -1;
  }

  if((thread_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0)
  {
		perror("socket failed");
  }

  if(connect(thread_socket, res->ai_addr, res->ai_addrlen) == -1)
  {
	printf("Connect to server failed\n");
  }
  else
  {
	  printf("Client connected with Server %s successfully.", argv[thread_id + 1]);
  }
  
  while(signal_flag)
  {
 	  
    char rcv_cmd[80]={0}; 
     
     //receive the string
 	ssize_t length = recv(thread_socket, rcv_cmd, sizeof(rcv_cmd), 0);
 	
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
    	 if(strcmp(rcv_cmd, "threshold reached") == 0)
    	 {
    		 send(socket_client, rcv_cmd, strlen(rcv_cmd), MSG_DONTWAIT);
    	 }
     }
 	
     close(fd);
  }
  pthread_exit((void *)0);
}
  
int main(int argc, char *argv[])
{
  thread_count = argv[1]-1;
  pthread_t thread[thread_count]; 
  threadParams_t threadParams[thread_count];
  signal(SIGTERM,handle_sig);
  signal(SIGINT,handle_sig);
  uint8_t user_threadid = argv[1]+1;
  
  if(pthread_mutex_init(&socklock, NULL) != 0)
  {
	  printf("mutex init failed.\n");
	  return -1; 
  }

  struct addrinfo ref, *res;
  memset(&ref, 0, sizeof(ref));
  ref.ai_family = AF_UNSPEC;
  ref.ai_socktype = SOCK_STREAM;
  
  printf("Client is trying to connect with %d servers.\n", argv[1]);
  printf("Client considers last IP address as user IP address by default.\n");

  if(getaddrinfo(argv[user_threadid], PORT, &ref, &res) != 0)
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
  else
  {
	  printf("Client connected with Server %s successfully.", argv[user_threadid]);
  }

  while(signal_flag)
  {
	  
	for(uint8_t i=0; i<thread_count; i++)
	{
		threadParams[i].threadIdx = i+1;
		
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
  printf("Signal received, shutting down...\n");
}
  //shutdown(socket_client,SHUT_RDWR);
  