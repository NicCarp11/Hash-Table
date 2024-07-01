
CC = gcc
CFLAGS = -pthread
TARGET = server client scalability_test

UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S), Linux)
    LDFLAGS = -lrt
else
    LDFLAGS =
endif

all: $(TARGET)

hashTable.o: hashTable.c hashTable.h
	$(CC) $(CFLAGS) -c hashTable.c

server: server.c hashTable.o
	$(CC) $(CFLAGS) -o server server.c hashTable.o $(LDFLAGS)

client: client.c
	$(CC) $(CFLAGS) -o client client.c $(LDFLAGS)

scalability_test: scalability_test.c hashTable.o
	$(CC) $(CFLAGS) -o scalability_test scalability_test.c hashTable.o $(LDFLAGS)

clean:
	rm -f $(TARGET) *.o

.PHONY: all clean
