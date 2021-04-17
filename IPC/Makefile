INCLUDE_DIRS = 
LIB_DIRS = 
CC=gcc

CDEFS=
CFLAGS= -O -g $(INCLUDE_DIRS) $(CDEFS) -DLINUX
LIBS= -pthread -lrt

HFILES= 
CFILES= mq_heap_rcv.c mq_heap_send.c 

SRCS= ${HFILES} ${CFILES}
OBJS= ${CFILES:.c=.o}

all:	mq_heap_rcv mq_heap_send

clean:
	-rm -f *.o *.d
	-rm -f perfmon mq_heap_send mq_heap_rcv

distclean:
	-rm -f *.o *.d
	-rm -f mq_heap_send mq_heap_rcv

mq_heap_send: mq_heap_send.o 
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $@.o $(LIBS)

mq_heap_rcv: mq_heap_rcv.o 
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $@.o $(LIBS)

depend:

.c.o:
	$(CC) $(CFLAGS) -c $<
