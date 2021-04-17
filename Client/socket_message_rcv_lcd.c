#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include "socket_lcd.h"

#include <wiringPi.h>

#define LCD_E 23
#define LCD_RS 22
#define LCD_D4 24
#define LCD_D5 25
#define LCD_D6 8
#define LCD_D7 7

#define SERVER_QUEUE_NAME   "/aesd-server"
#define QUEUE_PERMISSIONS 0660
#define MAX_MESSAGES 25
#define MAX_MSG_SIZE 256
#define MSG_BUFFER_SIZE MAX_MSG_SIZE + 10

void pulseEnable ()
{
   digitalWrite (LCD_E, HIGH) ; 
   delay(0.5); //  1/2 microsecond pause - enable pulse must be > 450ns
   digitalWrite (LCD_E, LOW) ; 
}

/*
  send a byte to the lcd in two nibbles
  before calling use SetChrMode or SetCmdMode to determine whether to send character or command
*/
void lcd_byte(char bits)
{
  digitalWrite (LCD_D4,(bits & 0x10)) ;  
  digitalWrite (LCD_D5,(bits & 0x20)) ;  
  digitalWrite (LCD_D6,(bits & 0x40)) ;  
  digitalWrite (LCD_D7,(bits & 0x80)) ;  
  pulseEnable();

  digitalWrite (LCD_D4,(bits & 0x1)) ;  
  digitalWrite (LCD_D5,(bits & 0x2)) ;  
  digitalWrite (LCD_D6,(bits & 0x4)) ;  
  digitalWrite (LCD_D7,(bits & 0x8)) ;  
  pulseEnable();      	
}

void SetCmdMode() 
{
  digitalWrite (LCD_RS, 0); // set for commands
}

void SetChrMode() 
{
  digitalWrite (LCD_RS, 1); // set for characters
}

void lcd_text(char *s)
{
  while(*s) 
	lcd_byte(*s++);
 }

void lcd_init()
{
   wiringPiSetupGpio () ; // use BCIM numbering
   // set up pi pins for output
   pinMode (LCD_E,  OUTPUT);
   pinMode (LCD_RS, OUTPUT);
   pinMode (LCD_D4, OUTPUT);
   pinMode (LCD_D5, OUTPUT);
   pinMode (LCD_D6, OUTPUT);
   pinMode (LCD_D7, OUTPUT);
   
   // initialise LCD
   SetCmdMode(); // set for commands
   lcd_byte(0x33); // full init 
   lcd_byte(0x32); // 4 bit mode
   lcd_byte(0x28); // 2 line mode
   lcd_byte(0x0C); // display on, cursor off, blink off
   lcd_byte(0x01);  // clear screen
   delay(3);        // clear screen is slow!
}

//int print_on_lcd(char *str) 
int lcd_print(char *msg) 
{
  //lcd_init();
  
  SetChrMode(); 
  lcd_text(msg);
  return 0 ;
}

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
    lcd_init();

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
        lcd_print(&in_buffer[i]);

        printf ("Server: response printed on lcd.\n");
        token_number++;
    }
}
