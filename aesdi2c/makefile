ifeq ($(CC),)
	CC = $(CROSS_COMPILE)gcc
endif
ifeq ($CFLAGS),)
	CFLAGS = -g -Wall -Werror
endif
ifeq ($(INCLUDES),)
	INCLUDES = -I/
endif

all: i2c

i2c.o: i2c.c
	$(CC) $(CFLAGS) -c i2c.c

clean:
	rm -f *.o
	rm -f i2c
