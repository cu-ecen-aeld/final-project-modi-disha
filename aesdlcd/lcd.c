#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <wiringPi.h>
#include <lcd.h>

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

#define IN  0
#define OUT 1

#define LOW  0
#define HIGH 1

#define PIN  24 /* P1-18 */
#define POUT1 5  /* P1-07 */
#define POUT2 6
#define POUT3 13
#define POUT4 19


int main (int argc, char **argv)
{

    	int lcd;
	wiringPiSetup();
	lcd = lcdInit(2, 16, 4, LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7, 0, 0, 0, 0);
	
	lcdPosition(lcd, 0, 0);
	lcdPrintf(lcd, "Hello, world!");
	//lcdPuts(lcd, "Hello, world!");

	sleep(2);
	sleep(20);

	lcdClear(lcd);
	lcdPosition(lcd, 0, 0);
	lcdPrintf(lcd, "Hi Girls, how are you?");
	sleep(2);
	
	//lcdClear(lcd);
}
