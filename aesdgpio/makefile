ifeq ($(CC),)
	CC = $(CROSS_COMPILE)gcc
endif
ifeq ($CFLAGS),)
	CFLAGS = -g -Wall -Werror
endif
ifeq ($(INCLUDES),)
	INCLUDES = -I/
endif

all: gpio gpio_module

gpio.o: gpio.c
	$(CC) $(CFLAGS) -c gpio.c

gpio_module.o: gpio_module.c
	$(CC) $(CFLAGS) -c gpio_module.c

clean:
	rm -f *.o
	rm -f gpio
