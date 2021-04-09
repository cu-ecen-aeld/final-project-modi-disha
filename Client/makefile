ifeq ($(CC),)
	CC = $(CROSS_COMPILE)gcc
endif
ifeq ($CFLAGS),)
	CFLAGS = -g -Wall -Werror
endif
ifeq ($(LDFLAGS),)
	LDFLAGS = -pthread -lrt
endif
ifeq ($(INCLUDES),)
	INCLUDES = -I/
endif

all: SocketClient SocketServer SocketServer1

SocketServer.o: SocketServer.c
	$(CC) $(CFLAGS) -c SocketServer.c

SocketServer: SocketServer.o
	$(CC) $(CFLAGS) $(INCLUDES) SocketServer.o -o SocketServer $(LDFLAGS)
	
SocketServer1.o: SocketServer1.c
	$(CC) $(CFLAGS) -c SocketServer1.c

SocketServer1: SocketServer1.o
	$(CC) $(CFLAGS) $(INCLUDES) SocketServer1.o -o SocketServer1 $(LDFLAGS)

SocketClient.o: SocketClient.c
	$(CC) $(CFLAGS) -c SocketClient.c

SocketClient: SocketClient.o
	$(CC) $(CFLAGS) $(INCLUDES) SocketClient.o -o SocketClient $(LDFLAGS)

clean:
	rm -f *.o
	rm -f SocketClient
	rm -f SocketServer SocketServer1
