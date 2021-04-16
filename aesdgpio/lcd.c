#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "lcd.h"

#define IN  0
#define OUT 1

#define LOW  0
#define HIGH 1
#define RS 2 /* P1-18 */
#define RW 3  /* P1-07 */
#define EN 17
#define D0 27
#define D1 22
#define D2 10
#define D3 9
#define D4 11
#define D5 5
#define D6 6
#define D7 13



#define ROW_0 0x00
#define ROW_1 0x40
#define ROW_2 0x10
#define ROW_3 0x50


#define VALUE_MAX 30

static int
GPIOExport(int pin)
{
#define BUFFER_MAX 3
char buffer[BUFFER_MAX];
ssize_t bytes_written;
int fd;

fd = open("/sys/class/gpio/export", O_WRONLY);
if (-1 == fd) {
	fprintf(stderr, "Failed to open export for writing!\n");
	return(-1);
}

bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
write(fd, buffer, bytes_written);
close(fd);
return(0);
}

// static int
// GPIOUnexport(int pin)
// {
// char buffer[BUFFER_MAX];
// ssize_t bytes_written;
// int fd;

// fd = open("/sys/class/gpio/unexport", O_WRONLY);
// if (-1 == fd) {
// 	fprintf(stderr, "Failed to open unexport for writing!\n");
// 	return(-1);
// }

// bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
// write(fd, buffer, bytes_written);
// close(fd);
// return(0);
// }

static int
GPIODirection(int pin, int dir)
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

// static int
// GPIORead(int pin)
// {
// #define VALUE_MAX 30
// char path[VALUE_MAX];
// char value_str[3];
// int fd;

// snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
// fd = open(path, O_RDONLY);
// if (-1 == fd) {
// 	fprintf(stderr, "Failed to open gpio value for reading!\n");
// 	return(-1);
// }

// if (-1 == read(fd, value_str, 3)) {
// 	fprintf(stderr, "Failed to read value!\n");
// 	return(-1);
// }

// close(fd);

// return(atoi(value_str));
// }

static int
GPIOWrite(int pin, int value)
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

int lcd_init()
{
  /*
	* Enable GPIO pins
	*/
if (-1 == GPIOExport(RS) || -1 == GPIOExport(EN) || -1 == GPIOExport(RW) || -1 == GPIOExport(D0)
    ||-1 == GPIOExport(D1) || -1 == GPIOExport(D2)|| -1 == GPIOExport(D3) || -1 == GPIOExport(D4)
    ||-1 == GPIOExport(D5) || -1 == GPIOExport(D6)||-1 == GPIOExport(D7))
	  return(1);

  if (-1 == GPIODirection(RS, OUT) || -1 == GPIODirection(RW, OUT)|| -1 == GPIODirection(EN, OUT)||-1 == GPIOExport(D1) || -1 == GPIOExport(D2)|| -1 == GPIOExport(D3) || -1 == GPIOExport(D4)
      ||-1 == GPIOExport(D5) || -1 == GPIOExport(D6)||-1 == GPIOExport(D7))
	    return(2);

  return(0);

}
int cmd_write(char data)
{


  if (-1 == GPIOWrite(RS, 0))
	  return(3);
  if (-1 == GPIOWrite(RW, 0))
	  return(3);
  data_out(data);

  return 0;

}
int data_wtite(char data)
{
  if (-1 == GPIOWrite(RS, 1))
  	return(3);
  if (-1 == GPIOWrite(RW, 0))
	  return(3);
  data_out(data);

  return 0;
}

/*********************************************************************
// Name: lcdgotoaddr()
// Description: Sets the cursor to the specified LCD DDRAM address.
// Should call lcdbusywait().
**************************************************************/
void lcdgotoaddr(unsigned char addr) {
  addr |= 0x80; //INCLUDE THE LAST BIT OF BUSY FLAG
  cmd_write (addr);
  usleep(2*1000);
}

/*****************************************************************************
// Name: lcdgotoxy()
// Description: Sets the cursor to the LCD DDRAM address corresponding
// to the specified row and column. Location (0,0) is the top left
// corner of the LCD screen. Must call lcdgotoaddr().
**************************************************************************/
void lcdgotoxy(unsigned char row, unsigned char column) {
  unsigned char address;
  if (row == 0) {
    address = ROW_0 | column; //CONCATENATE TO FORM ADDRESS
    lcdgotoaddr(address);

  } else if (row == 1) {
    address = ROW_1 | column;
    lcdgotoaddr(address);
  } else if (row == 2) {
    address = ROW_2 | column;
    lcdgotoaddr(address);
  } else if (row == 3) {
    address = ROW_3 | column;
    lcdgotoaddr(address);
  }

}

int data_out(char data)
{
  if(data & 0x01)
    GPIOWrite(D0,1);
  else
   GPIOWrite(D0,0);
  
  if(data & 0x02)
    GPIOWrite(D1,1);
  else
   GPIOWrite(D1,0);

  if(data & 0x04)
    GPIOWrite(D2,1);
  else
   GPIOWrite(D2,0);

  if(data & 0x08)
    GPIOWrite(D3,1);
  else
   GPIOWrite(D3,0);

  if(data & 0x10)
    GPIOWrite(D4,1);
  else
   GPIOWrite(D4,0);

  if(data & 0x20)
    GPIOWrite(D5,1);
  else
   GPIOWrite(D5,0);

  if(data & 0x40)
    GPIOWrite(D6,1);
  else
   GPIOWrite(D6,0);

  if(data & 0x80)
    GPIOWrite(D7,1);
  else
   GPIOWrite(D7,0);

  GPIOWrite(EN,1);
  usleep(500);
  GPIOWrite(EN,0);
  usleep(50);

return 0;

}

/******************************************************************************
// Name: lcdputch()
// Description: Writes the specified character to the current
// LCD cursor position. Should call lcdbusywait().
***********************************************************************/
void lcdputch(char cc) {
  data_wtite (cc);
  usleep(2*1000);
}
/******************************************************************************
// Name: lcdputstr()
// Description: Writes the specified null-terminated string to the LCD
// starting at the current LCD cursor position. Automatically wraps
// long strings to the next LCD line after the right edge of the
// display screen has been reached. Must call lcdputch().
******************************************************************************/
void lcdputstr(char * ss) {
    lcdgotoxy(1,0);
  // char address = IR_READ & 0X7F; //READ CURRENT POSITION
  // delay_us(20);

  // unsigned char row_read = address & 0XF0; //GET CURRENT ROW
  // unsigned char col_read = address & 0X0F;//GET CURRENT COLUMN

  // Loop while the STRING isn't null
  while ( * ss) {
  //   //IF COLUMN REACHES 0XF THAT IS THE LAST PIXEL GO TO NEXT ROW
  //   if (col_read > 0xF) {
  //     if (row_read == ROW_0) {
  //       lcdgotoxy(1, 0);
  //       col_read = 0;
  //     } else if (row_read == ROW_1) {
  //       lcdgotoxy(2, 0);
  //       col_read = 0;
  //     }

  //   }

    lcdputch( * ss++);

  }
}

int lcd_start()
{
  usleep(20*1000);
  cmd_write ( 0x30); //Unlock Command //BF CANNOT BE CHECKED SO GIVE DELAY MORE THAN INSTRUCTION
  usleep(1000*5);

  cmd_write (0x30);//Unlock Command //BF CANNOT BE CHECKED SO GIVE DELAY MORE THAN INSTRUCTION
  usleep(150);

  cmd_write (0x30);//Unlock Command //BF CANNOT BE CHECKED SO GIVE DELAY MORE THAN INSTRUCTION
  usleep(1000*1);

  usleep(2*1000);
  cmd_write ( 0x38); //Function set (Interface is 8 bits long. Specify the number of display lines and character font.)
                    // N=1, F=0

  usleep(20);

  usleep(2*1000);

  cmd_write ( 0x08);  //Turn the display OFF
  usleep(20);

  usleep(2*1000);//POLL THE BUSY FLAG

  cmd_write ( 0x0C); //Turn the display OFF
  usleep(20);

  usleep(2*1000);//POLL THE BUSY FLAG

 //poll the busy flag

 cmd_write (0x06);//Entry Mode Set command
  usleep(1000*1);

  usleep(2*1000);//POLL THE BUSY FLAG
  cmd_write ( 0x01); //Clear screen and send the cursor home.
  usleep(20);

 return(0);
}


int main()
{
  lcd_init();
  lcd_start();
  lcdputstr("Hello");
  return(0);
}