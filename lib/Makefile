LIBS := libskiplist.a

CC = gcc
INCDIR = ../include/
CFLAGS += -Wall -O2 -I$(INCDIR)
LDFLAGS += -lpthread

.PHONY: all

all: $(LIBS)

libskiplist.a: skiplist.c $(INCDIR)/skiplist.h
	$(CC) $(CFLAGS) $(LDFLAGS) -c skiplist.c
	ar -cvq libskiplist.a skiplist.o

clean:
	rm *.o *.a
