ifeq ($(CC),)
	CC = $(CROSS_COMPILE)gcc
endif
ifeq ($CFLAGS),)
	CFLAGS = -g -Wall -Werror
endif
ifeq ($(LDFLAGS),)
	LDFLAGS = -lwiringPi -lwiringPiDev -pthread -lrt
endif
ifeq ($(INCLUDES),)
	INCLUDES = -I/
endif

all: lcd

lcd.o: lcd.c
	$(CC) $(CFLAGS) -c lcd.c

lcd: lcd.o
	$(CC) $(CFLAGS) $(INCLUDES) lcd.o -o lcd $(LDFLAGS)

clean:
	rm -f *.o
	rm -f lcd
