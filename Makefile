CC=g++
CFLAGS=-Werror -Wall -std=c++11 -g
INCLUDES=-lpcap
OBJFOLDER=obj
DEPSFOLDER=inc
DEPS=$(DEPSFOLDER)/*
SOURCEFOLDER=src
BINFOLDER=bin

$(OBJFOLDER)/%.o: $(SOURCEFOLDER)/%.cpp
	[ -d ./$(OBJFOLDER) ] || mkdir $(OBJFOLDER)
	$(CC) -c $< $(CFLAGS) -o $@

all:	server client

server: $(OBJFOLDER)/Mac.o $(OBJFOLDER)/Bridge.o $(OBJFOLDER)/SpanningTree.o $(OBJFOLDER)/Server.o $(OBJFOLDER)/serverMain.o
	[ -d $(BINFOLDER) ] || mkdir $(BINFOLDER)
	$(CC) $(CFLAGS) $^ -o $(BINFOLDER)/server

client: $(OBJFOLDER)/Mac.o $(OBJFOLDER)/Bridge.o $(OBJFOLDER)/Sniffer.o $(OBJFOLDER)/SpanningTree.o $(OBJFOLDER)/Client.o $(OBJFOLDER)/clientMain.o
	[ -d $(BINFOLDER) ] || mkdir $(BINFOLDER)
	$(CC) $(CFLAGS) $^ $(INCLUDES) -o $(BINFOLDER)/client

clean:
	rm -r $(BINFOLDER) $(OBJFOLDER)
