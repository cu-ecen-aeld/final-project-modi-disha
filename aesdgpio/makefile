ifeq ($(CC),)
	CC = $(CROSS_COMPILE)gcc
endif
ifeq ($CFLAGS),)
	CFLAGS = -g -Wall -Werror
endif
ifeq ($(INCLUDES),)
	INCLUDES = -I/
endif

all: gpio

gpio.o: gpio.c
	$(CC) $(CFLAGS) -c gpio.c

clean:
	rm -f *.o
	rm -f gpio
