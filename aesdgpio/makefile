ifeq ($(CC),)
	CC = $(CROSS_COMPILE)gcc
endif
ifeq ($CFLAGS),)
	CFLAGS = -g -Wall -Werror
endif
ifeq ($(INCLUDES),)
	INCLUDES = -I/
endif

all: gpio gpio_module gpio_led

gpio.o: gpio.c
	$(CC) $(CFLAGS) -c gpio.c

gpio_module.o: gpio_module.c
	$(CC) $(CFLAGS) -c gpio_module.c

gpio_led.o: gpio_led.c
	$(CC) $(CFLAGS) -c gpio_led.c

clean:
	rm -f *.o
	rm -f gpio gpio_led gpio_module
