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

all: SocketClient SocketServer socket_lcd socket_message_rcv socket_message_rcv_lcd

SocketServer.o: SocketServer.c
	$(CC) $(CFLAGS) -c SocketServer.c

SocketServer: SocketServer.o
	$(CC) $(CFLAGS) $(INCLUDES) SocketServer.o -o SocketServer $(LDFLAGS)

SocketClient.o: SocketClient.c
	$(CC) $(CFLAGS) -c SocketClient.c

SocketClient: SocketClient.o
	$(CC) $(CFLAGS) $(INCLUDES) SocketClient.o -o SocketClient $(LDFLAGS)

SocketClient.o: SocketClient.c
	$(CC) $(CFLAGS) -c SocketClient.c

SocketClient: SocketClient.o
	$(CC) $(CFLAGS) $(INCLUDES) SocketClient.o -o SocketClient $(LDFLAGS)

socket_lcd.o: socket_lcd.c
	$(CC) $(CFLAGS) -c socket_lcd.c

socket_lcd: socket_lcd.o
	$(CC) $(CFLAGS) $(INCLUDES) socket_lcd.o -o socket_lcd -lwiringPi -lwiringPiDev -pthread -lrt

socket_message_rcv.o: socket_message_rcv.c
	$(CC) $(CFLAGS) -c socket_message_rcv.c

socket_message_rcv: socket_message_rcv.o
	$(CC) $(CFLAGS) $(INCLUDES) socket_message_rcv.o -o socket_message_rcv -lrt -pthread

socket_message_rcv_lcd.o: socket_message_rcv_lcd.c
	$(CC) $(CFLAGS) -c socket_message_rcv_lcd.c

socket_message_rcv_lcd: socket_message_rcv_lcd.o
	$(CC) $(CFLAGS) $(INCLUDES) socket_message_rcv_lcd.o -o socket_message_rcv_lcd -lwiringPi -lwiringPiDev -pthread -lrt

clean:
	rm -f *.o
	rm -f SocketClient
	rm -f SocketServer 
