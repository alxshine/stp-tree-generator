CC=gcc
CFLAGS=-Werror -Wall -g
LIBS=-lpcap -pthread

all: stp.c
	$(CC) $(CFLAGS) $< $(LIBS) -o switch
