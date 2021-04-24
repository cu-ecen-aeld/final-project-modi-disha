#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <wiringPi.h>

#define LCD_E 23
#define LCD_RS 22
#define LCD_D4 24
#define LCD_D5 25
#define LCD_D6 8
#define LCD_D7 7

#define IN  0
#define OUT 1

#define LOW  0
#define HIGH 1

#define PIN  24 /* P1-18 */
#define POUT1 5  /* P1-07 */
#define POUT2 6
#define POUT3 13
#define POUT4 19

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

static int
GPIOUnexport(int pin)
{
	char buffer[BUFFER_MAX];
	ssize_t bytes_written;
	int fd;

	fd = open("/sys/class/gpio/unexport", O_WRONLY);
	if (-1 == fd) {
		fprintf(stderr, "Failed to open unexport for writing!\n");
		return(-1);
	}

	bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
	write(fd, buffer, bytes_written);
	close(fd);
	return(0);
}

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

static int
GPIORead(int pin)
{
#define VALUE_MAX 30
	char path[VALUE_MAX];
	char value_str[3];
	int fd;

	snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
	fd = open(path, O_RDONLY);
	if (-1 == fd) {
		fprintf(stderr, "Failed to open gpio value for reading!\n");
		return(-1);
	}

	if (-1 == read(fd, value_str, 3)) {
		fprintf(stderr, "Failed to read value!\n");
		return(-1);
	}

	close(fd);

	return(atoi(value_str));
}

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

int GPIOinit()
{

	if (-1 == GPIOExport(POUT1) || -1 == GPIOExport(POUT2) || -1 == GPIOExport(POUT3) || -1 == GPIOExport(POUT4))
		return(1);

	/*
	 * Set GPIO directions
	 */
	if (-1 == GPIODirection(POUT1, OUT) || -1 == GPIODirection(POUT2, OUT) || -1 == GPIODirection(POUT3, OUT) || -1 == GPIODirection(POUT4, OUT))
		return(2);


}

int gpio_write(int pin)
{
	if (-1 == GPIOWrite(pin, 1))
			return(3);
	
        sleep(1);

	if (-1 == GPIOWrite(pin, 0))
			return(3);
}

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
  lcd_byte(0x01);  //Clear screen
  sleep(2);        // clear screen is slow!
  SetChrMode(); 
  lcd_text(msg);
  return 0 ;
}

int main (int argc, char **argv)
{

    printf ("Server: Hello, World!\n");
    char *out_buffer ;

   // GPIOinit();
    lcd_init();

    while (1) 
    {
        // get the oldest message with highest priority
 

        printf ("Server: message received.\n");

	lcd_print("Hello Maitreyee!");
	printf ("Hello Maitreyee!");

	lcd_print("Hello Swati!");
	printf ("Hello Swati!");
	
	lcd_print("Hello Girls!");
	printf ("Hello Girls!");

        printf ("Server: response printed on lcd.\n");
    }
}
