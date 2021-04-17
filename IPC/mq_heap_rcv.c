 /*****************************************************************************
 * Copyright (C) 2020 by Disha Modi
 * Redistribution, modification or use of this software in source or binary
 * forms is permitted as long as the files maintain this copyright. Users are
 * permitted to modify this and use it to learn about the field of embedded
 * software. Disha Modi, and the University of Colorado are not liable for
 * any misuse of this material.
 *
******************************************************************************/
/**
 * @file mq_heap.c
 * @brief An abstraction for message queue to share the global data/msg between threads.
 *
 * This source file includes VxWorks heap_mq.c equivalent code written with Linux NPTL threads.
 * This code has message sender and receiver threads. Sender thread puts pointer to message buffer 
 * on POSIX message queue and receiver thread reads message pointer from the same message queue.  
 *
 * Tools​ ​ used​ ​ to​ ​ process​ ​ the​ ​ code​ ​ (OS, compiler,​ ​ linker,​ ​ debugger):
 * Linux kernel 4.19.118-v7+, Raspbian GNU/Linux 10 (buster) OS, 
 * GNU C compiler and linker, Gnu Make Builder 
 *
 * Identification​ ​ of​ ​ any​ ​ leveraged​ ​ code:
 * @ref http://ecee.colorado.edu/~ecen5623/ecen/ex/Linux/code/VxWorks-Examples/heap_mq.c
 *
 * @author Disha Modi
 * @date July 2020
 * @version 1.0
 *******************************************************************************/

/*******************************************************************************
 * Include files
 ******************************************************************************/

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <mqueue.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <syslog.h>

/// Definitions
#define SNDRCV_MQ "/send_receive_mq"
#define ERROR (-1)
#define NUM_THREADS (1)     // service threads
#define NUM_CPU_CORES (1)
#define CPU_CORE 3
#define COUNT 1

sem_t send, receive;	/// semaphores
struct mq_attr mq_attr;
static mqd_t mymq;   /// Message queue
static char imagebuff[4096];   /// Message buffer

typedef struct
{
    int threadIdx;
    int MajorPeriods;
} threadParams_t;

/*!
 * @brief receiver thread reads pointer to heap from the message queue
 * receives pointer to heap, reads it, and deallocate heap memory
 */
void *receiver()
{
  char buffer[sizeof(void *)+sizeof(int)]; /// Buffer to receive heap pointer
  void *buffptr = NULL;  /// Pointer to dereference the heap buffer
  int prio;
  int nbytes;
  int count = 0;
  int id, i = 0;
  
  //sem_wait(&receive);
  syslog(LOG_CRIT, "Entered receiver thread.\n");
 
  while(i < COUNT) 
  {    
    syslog(LOG_CRIT, "Reading %ld bytes\n", sizeof(void *)+sizeof(int));
  
    if((nbytes = mq_receive(mymq, buffer, (size_t)(sizeof(void *)+sizeof(int)), &prio)) == ERROR)
    {
      perror("mq_receive");
    }
    else
    {
	  /// Copy the pointer address and message id	
      memcpy(buffptr, buffer, sizeof(void *));
      memcpy((void *)&id, &(buffer[sizeof(void *)]), sizeof(int));

      syslog(LOG_CRIT, "receive: ptr msg 0x%p received with priority = %d, length = %d, id = %d\n", buffptr, prio, nbytes, id);
		
      syslog(LOG_CRIT, "Message Received = %s", (char *)buffptr);
      syslog(LOG_CRIT, "\n");

    }
    i++;
  }

    free(buffptr);  /// free the heap buffer
    syslog(LOG_CRIT, "heap space memory freed\n");
}

/*!
 * @brief sender thread puts the pointer to heap memory on the message queue
 */ 
void *sender()
{
  char buffer[sizeof(void *)+sizeof(int)];
  void *buffptr;
  int prio = 30;
  int nbytes;
  int id = 999, i = 0;
  buffptr = (void *)malloc(sizeof(imagebuff));
  memcpy(buffptr, imagebuff, sizeof(imagebuff));

  while(i < COUNT) 
  {	
    syslog(LOG_CRIT, "Message to send = %s", (char *)buffptr);

    syslog(LOG_CRIT, "Sending %ld bytes\n", sizeof(void *)+sizeof(int));

    /// Copy the pointer address and message id to sending buffer
    memcpy(buffer, &buffptr, sizeof(void *));
    memcpy(&(buffer[sizeof(void *)]), (void *)&id, sizeof(int));

    if((nbytes = mq_send(mymq, buffer, (size_t)(sizeof(void *)+sizeof(int)), prio)) == ERROR)
    {
      perror("mq_send");
    }
    else
    {
      syslog(LOG_CRIT, "send: message ptr 0x%X sent with priority = %d, length = %ld, id = %d\n", buffptr, prio, (sizeof(void *)+sizeof(int)), id);
    }
    i++;
    
  }

}

/*!
 * @brief Prints the scheduling policy for the calling process.
 */ 
void print_scheduler(void)
{
   int schedType;

   schedType = sched_getscheduler(getpid());

   switch(schedType)
   {
     case SCHED_FIFO:
           syslog(LOG_CRIT, "Pthread Policy is SCHED_FIFO\n");
           break;
     case SCHED_OTHER:
           syslog(LOG_CRIT, "Pthread Policy is SCHED_OTHER\n"); exit(-1);
       break;
     case SCHED_RR:
           syslog(LOG_CRIT, "Pthread Policy is SCHED_RR\n"); exit(-1);
           break;
     default:
       syslog(LOG_CRIT, "Pthread Policy is UNKNOWN\n"); exit(-1);
   }

}

static int sid, rid;

/*!
 * @brief The program starts with main routine. This thread creates the 
 * sender and receiver thread and schedules them.
 */ 
void main()
{
	int i, j;
	char pixel = 'A';

	int rc, scope;
	cpu_set_t threadcpu;
	pthread_t threads[NUM_THREADS];
	threadParams_t threadParams[NUM_THREADS];
	pthread_attr_t rt_sched_attr[NUM_THREADS];
	int rt_max_prio, rt_min_prio;
	struct sched_param rt_param[NUM_THREADS];
	struct sched_param main_param;
	pthread_attr_t main_attr;
	pid_t mainpid;
	cpu_set_t allcpuset;

	for(i=0;i<4096;i+=64) {
		pixel = 'A';
		for(j=i;j<i+64;j++) {
			imagebuff[j] = (char)pixel++;
		}
		imagebuff[j-1] = ' ';
	}
	imagebuff[4096] = '\0';
	//  imagebuff[63] = '\0';

	syslog(LOG_CRIT, "buffer = %s", imagebuff);

	/* setup common message q attributes */
	mq_attr.mq_flags = 0;
	mq_attr.mq_maxmsg = 1;
	mq_attr.mq_msgsize = sizeof(void *) + sizeof(int);
	mq_attr.mq_curmsgs = 0;

	/* Message queue open with readwrite permissions and blocking mode */
	mymq = mq_open(SNDRCV_MQ, O_CREAT|O_RDWR, 0644, &mq_attr);

	if(mymq == (mqd_t)ERROR)
		perror("mq_open");

/*	CPU_ZERO(&allcpuset);

	for(i=0; i < NUM_CPU_CORES; i++)
	   CPU_SET(i, &allcpuset);

	CPU_ZERO(&threadcpu);
	CPU_SET(CPU_CORE, &threadcpu);
	syslog(LOG_CRIT, "Using CPUS=%d from total available.\n", CPU_COUNT(&allcpuset));

	/// initialize the sequencer semaphores
	if (sem_init (&send, 0, 0)) { printf ("Failed to initialize send semaphore\n"); exit (-1); }
	if (sem_init (&receive, 0, 0)) { printf ("Failed to initialize receive semaphore\n"); exit (-1); }

	mainpid=getpid();

   /// max and min prioritis of SCHED_FIFO 
	rt_max_prio = sched_get_priority_max(SCHED_FIFO);
	rt_min_prio = sched_get_priority_min(SCHED_FIFO);

	rc=sched_getparam(mainpid, &main_param);
	main_param.sched_priority=rt_max_prio;
	rc=sched_setscheduler(getpid(), SCHED_FIFO, &main_param);
	if(rc < 0) perror("main_param");

	print_scheduler();

	pthread_attr_getscope(&main_attr, &scope);

	if(scope == PTHREAD_SCOPE_SYSTEM)
	  syslog(LOG_CRIT, "PTHREAD SCOPE SYSTEM\n");
	else if (scope == PTHREAD_SCOPE_PROCESS)
	  syslog(LOG_CRIT, "PTHREAD SCOPE PROCESS\n");
	else
	  syslog(LOG_CRIT, "PTHREAD SCOPE UNKNOWN\n");

	syslog(LOG_CRIT, "rt_max_prio=%d\n", rt_max_prio);
	syslog(LOG_CRIT, "rt_min_prio=%d\n", rt_min_prio); */

	for(i=0; i < NUM_THREADS; i++)
	{
	  /// thread attributes initialization
	/*  rc=pthread_attr_init(&rt_sched_attr[i]);
	  rc=pthread_attr_setinheritsched(&rt_sched_attr[i], PTHREAD_EXPLICIT_SCHED);
	  rc=pthread_attr_setschedpolicy(&rt_sched_attr[i], SCHED_FIFO);
	  rc=pthread_attr_setaffinity_np(&rt_sched_attr[i], sizeof(cpu_set_t), &threadcpu);

	  rt_param[i].sched_priority=rt_max_prio - i;
	  pthread_attr_setschedparam(&rt_sched_attr[i], &rt_param[i]); */

	  threadParams[i].threadIdx=i;
	}

/*	syslog(LOG_CRIT, "Service threads will run on %d CPU cores\n", CPU_COUNT(&threadcpu));
	if(!CPU_ISSET(CPU_CORE, &threadcpu))
	{
		perror("Affinity Error:");
	}   */

	/// Create Service threads which will block awaiting release for:
	
	/// service MSG RECEIVE, at higher priority compared to MSG SEND service
	rc=pthread_create(&threads[0],               // pointer to thread descriptor
					          // use specific attributes
					  (void *)0,                 // default attributes
					  receiver,                     // thread function entry point
					  (void *)&(threadParams[0]) // parameters to pass in
					 );

	//syslog(LOG_CRIT, "MSG Receive thread created with priority %d on CPU core %d\n",  rt_param[0].sched_priority, CPU_CORE);                

	/// service MSG SEND
	//rc=pthread_create(&threads[1],               // pointer to thread descriptor
					//  &rt_sched_attr[1],         // use specific attributes
					  //(void *)0,                 // default attributes
					//  sender,                     // thread function entry point
					//  (void *)&(threadParams[1]) // parameters to pass in
					// );
	//pthread_attr_getaffinity_np(&rt_sched_attr[1], sizeof(cpu_set_t), &threadcpu);
	//syslog(LOG_CRIT, "MSG Send thread created with priority %d on CPU core %d\n",  rt_param[1].sched_priority, CPU_CORE);    

	/// Create two communicating processes right here
	//sem_post(&send); sem_post(&receive);

	for(i=0;i<NUM_THREADS;i++)
	   pthread_join(threads[i], NULL);

	if(mq_unlink(SNDRCV_MQ) == 0)
	  syslog(LOG_CRIT, "Message queue removed from the system.\n");

	syslog(LOG_CRIT, "TEST COMPLETE\n");

	}
