LIBS := libskiplist.a libbst.a

CC = gcc
INCDIR = ../include/
CFLAGS += -g -Wall -O2 -I$(INCDIR)
LDFLAGS += -lpthread

.PHONY: all

all: $(LIBS)

libskiplist.a: skiplist.c $(INCDIR)/skiplist.h
	$(CC) $(CFLAGS) $(LDFLAGS) -c skiplist.c
	ar -cvq libskiplist.a skiplist.o

libbst.a: bst.c $(INCDIR)/bst.h
	$(CC) $(CFLAGS) $(LDFLAGS) -c bst.c
	ar -cvq libbst.a bst.o

clean:
	rm *.o *.a
