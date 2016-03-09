CC=g++
CFLAGS=-Werror -Wall -std=c++11
INCLUDES=-lpcap
OBJFOLDER=obj
OBJ=$(OBJFOLDER)/stp_sniffer.c
DEPSFOLDER=inc
DEPS=$(DEPSFOLDER)/*
SOURCEFOLDER=src
BINFOLDER=bin

$(OBJFOLDER)/%.o: $(SOURCEFOLDER)/%.cpp
	$(CC) -c $< $(CFLAGS) -o $@

all: $(OBJFOLDER)/main.o $(OBJFOLDER)/Mac.o $(OBJFOLDER)/Bridge.o $(OBJFOLDER)/Sniffer.o
	$(CC) $(CFLAGS) $^ $(INCLUDES) -o $(BINFOLDER)/stpGen
