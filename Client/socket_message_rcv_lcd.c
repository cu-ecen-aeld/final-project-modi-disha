#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include "socket_lcd.h"

#define SERVER_QUEUE_NAME   "/aesd-server"
#define QUEUE_PERMISSIONS 0660
#define MAX_MESSAGES 25
#define MAX_MSG_SIZE 256
#define MSG_BUFFER_SIZE MAX_MSG_SIZE + 10

int main (int argc, char **argv)
{
    mqd_t lcdqueue;   // queue descriptors
    long token_number = 1; // next token to be given to client

    printf ("Server: Hello, World!\n");

    struct mq_attr attr;

    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;

    if ((lcdqueue = mq_open (SERVER_QUEUE_NAME, O_RDONLY | O_CREAT, QUEUE_PERMISSIONS, &attr)) == -1) {
        perror ("Server: mq_open (server)");
        exit (1);
    }
    char in_buffer [MSG_BUFFER_SIZE];
    char out_buffer [MSG_BUFFER_SIZE];

    while (1) {
        // get the oldest message with highest priority
        if (mq_receive (lcdqueue, in_buffer, MSG_BUFFER_SIZE, NULL) == -1) {
            perror ("Server: mq_receive");
            exit (1);
        }

        printf ("Server: message received %s.\n", in_buffer);

	int i = 0;
	for(i=0; i<strlen(in_buffer); i++)
	{
		if(in_buffer[i] == '!')
		{
			break;
		}
	}
        lcd_print(in_buffer[i]);

        printf ("Server: response printed on lcd.\n");
        token_number++;
    }
}
