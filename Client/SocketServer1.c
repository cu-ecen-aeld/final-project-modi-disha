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
#define PORTNO 9001
#define PORT "9001"

#define BACKLOG 10
int sockfd, newfd;
char s[INET6_ADDRSTRLEN];
int operation_switch =1;

//Signal HAndler function
void handle_sig(int sig)
{
  operation_switch=0;
  if(sig == SIGINT)
    syslog(LOG_DEBUG,"Caught SIGINT Signal exiting\n");
  if(sig == SIGTERM)
    syslog(LOG_DEBUG,"Caught SIGTERM Signal exiting\n");  
  shutdown(newfd,SHUT_RDWR);
  _exit(0);
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

//int opt;
//
///// start daemon if -d given in argument
//if((opt = getopt(argc, argv, "d")) == 'd')
//{
//	daemon(0, sockfd);
//}


	/// socket listen
	if(listen(sockfd, BACKLOG) < 0)
	{
	  syslog(LOG_ERR, "Socket listen failed.");
	  return -1;
	}
  
	int i = 0;

  while(operation_switch)
  {
	  
	  
		socklen_t addr_size = sizeof(opp_addr);
		
		/// accept socket connection
	newfd = accept(sockfd, (struct sockaddr *)&opp_addr, &addr_size);
	if(newfd < 0)
	{
			syslog(LOG_ERR, "Socket accept failed.");
		if(!operation_switch) 
		{
			break;
		}
         exit (EXIT_FAILURE);
	}
		
	inet_ntop(opp_addr.ss_family, get_in_addr((struct sockaddr *)&opp_addr),
                  s, sizeof s);
	
	while(1)
	{
		printf("index: %d\n", i++);


		/// LOG MSG TO SYSLOG: "Accepted connection from XXX"
		syslog(LOG_INFO, "Accepted connection from %s\n", s);
		
		char rdBuff[20] = "a = 50\n";
		
		int rc = send(newfd, rdBuff, strlen(rdBuff), MSG_DONTWAIT);
	
		if( rc < 0){
		  perror("Couldnt send sensor results to file\n");
		}
	
		usleep(500);
	}
 }
}
  //shutdown(socket_client,SHUT_RDWR);
  