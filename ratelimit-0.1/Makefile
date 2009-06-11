CFLAGS= -Wall -O2
CC=gcc

OBJS=ratelimit.o
#LIBS=-ldbm 
#LIBS=-lcrypt
LIBS=

all: ratelimit

ratelimit: ratelimit.o ipsum.o
	$(CC) $(CFLAGS) -o ratelimit ratelimit.o ipsum.o

tar:  clean
	cd .. ; tar -zcf ratelimit.tgz ratelimit

clean:
	rm -f ratelimit *.o
