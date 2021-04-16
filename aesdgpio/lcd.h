
static int GPIOExport(int pin);

//static int GPIOUnexport(int pin);

static int GPIODirection(int pin, int dir);

// static int
// GPIORead(int pin);


static int
GPIOWrite(int pin, int value);

int lcd_init();

int cmd_write(char data);

int data_wtite(char data);

void lcdgotoaddr(unsigned char addr);

void lcdgotoxy(unsigned char row, unsigned char column);

int data_out(char data);

void lcdputch(char cc) ;

void lcdputstr(char * ss);

int lcd_start();