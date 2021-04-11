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

all: SocketClient SocketServer

SocketServer.o: SocketServer.c
	$(CC) $(CFLAGS) -c SocketServer.c

SocketServer: SocketServer.o
	$(CC) $(CFLAGS) $(INCLUDES) SocketServer.o -o SocketServer $(LDFLAGS)

SocketClient.o: SocketClient.c
	$(CC) $(CFLAGS) -c SocketClient.c

SocketClient: SocketClient.o
	$(CC) $(CFLAGS) $(INCLUDES) SocketClient.o -o SocketClient $(LDFLAGS)

clean:
	rm -f *.o
	rm -f SocketClient
	rm -f SocketServer 
