ifeq ($(CC),)
	CC = $(CROSS_COMPILE)gcc
endif
ifeq ($CFLAGS),)
	CFLAGS = -g -Wall -Werror
endif
ifeq ($(LDFLAGS),)
	LDFLAGS = -pthread -lm
endif
ifeq ($(INCLUDES),)
	INCLUDES = -I/
endif

all: i2c i2c2

i2c.o: i2c.c
	$(CC) $(CFLAGS) -c i2c.c

i2c2.o: i2c2.c
	$(CC) $(CFLAGS) -c i2c2.c

i2c_imu: i2c_imu.c 
	$(CC) $(CFLAGS) i2c_imu.c MadgwickAHRS.c -o i2c_imu $(LDFLAGS)

clean:
	rm -f *.o
	rm -f i2c i2c2 i2c_imu
