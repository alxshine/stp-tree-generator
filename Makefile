CC=g++
CFLAGS=-Werror -Wall -std=c++11
INCLUDES=-lpcap
OBJFOLDER=obj
OBJ=$(OBJFOLDER)/stp_sniffer.c
DEPSFOLDER=inc
DEPS=Bridge.hpp
SOURCEFOLDER=src
BINFOLDER=bin

$(OBJFOLDER)/%.o: $(SOURCEFOLDER)/%.c
	$(CC) -c $< $(CFLAGS) -o $@

all: $(OBJFOLDER)/stp_sniffer.o
	$(CC) $(CFLAGS) $^ $(INCLUDES) -o $(BINFOLDER)/stpGen
