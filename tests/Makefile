CC = gcc
INCDIR = ../include/
LIBDIR = ../lib/
CFLAGS += -g -Wall -O2 -I$(INCDIR)
LDFLAGS += -L$(LIBDIR)
LDFLAGS += -lpthread -lskiplist -lbst -lpopt -lgmp

.PHONY: all

all: tests

# run_tests: tests
# 	@echo "Running all tests..."
# 	./tests

tests: test.c test.h
	$(CC) -o tests test.c $(CFLAGS) $(LDFLAGS)

clean:
	rm tests
