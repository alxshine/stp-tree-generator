CC=g++
CFLAGS=-Werror -Wall -g -std=c++11
INCLUDES=-lpcap
OBJFOLDER=obj
DEPSFOLDER=inc
DEPS=$(DEPSFOLDER)/*
SOURCEFOLDER=src
BINFOLDER=bin

$(OBJFOLDER)/%.o: $(SOURCEFOLDER)/%.cpp
	[ -d ./$(OBJFOLDER) ] || mkdir $(OBJFOLDER)
	$(CC) -c $< $(CFLAGS) -o $@

all:	server client parser test

server: $(OBJFOLDER)/Mac.o $(OBJFOLDER)/Bridge.o $(OBJFOLDER)/SpanningTree.o $(OBJFOLDER)/Server.o $(OBJFOLDER)/serverMain.o $(OBJFOLDER)/jsoncpp.o
	[ -d $(BINFOLDER) ] || mkdir $(BINFOLDER)
	$(CC) $(CFLAGS) $^ -o $(BINFOLDER)/server

client: $(OBJFOLDER)/Mac.o $(OBJFOLDER)/Bridge.o $(OBJFOLDER)/Sniffer.o $(OBJFOLDER)/SpanningTree.o $(OBJFOLDER)/Client.o $(OBJFOLDER)/clientMain.o $(OBJFOLDER)/jsoncpp.o
	[ -d $(BINFOLDER) ] || mkdir $(BINFOLDER)
	$(CC) $(CFLAGS) $^ $(INCLUDES) -o $(BINFOLDER)/client

sniffer: $(SOURCEFOLDER)/stp_sniffer.c
	[ -d $(BINFOLDER) ] || mkdir $(BINFOLDER)
	$(CC) $(CFLAGS) $^ $(INCLUDES) -o $(BINFOLDER)/stp_sniffer

parser: $(OBJFOLDER)/Mac.o $(OBJFOLDER)/Bridge.o $(OBJFOLDER)/Sniffer.o $(OBJFOLDER)/SpanningTree.o $(OBJFOLDER)/Client.o $(OBJFOLDER)/parser.o $(OBJFOLDER)/jsoncpp.o
	[ -d $(BINFOLDER) ] || mkdir $(BINFOLDER)
	$(CC) $(CFLAGS) $^ $(INCLUDES) -o $(BINFOLDER)/parser

test: $(OBJFOLDER)/Mac.o $(OBJFOLDER)/Bridge.o $(OBJFOLDER)/Sniffer.o $(OBJFOLDER)/SpanningTree.o $(OBJFOLDER)/Client.o $(OBJFOLDER)/jsoncpp.o $(OBJFOLDER)/test.o
	[ -d $(BINFOLDER) ] || mkdir $(BINFOLDER)
	$(CC) $(CFLAGS) $^ $(INCLUDES) -o $(BINFOLDER)/test

clean:
	[ -d $(BINFOLDER) ] && rm -r $(BINFOLDER) || echo "no $(BINFOLDER)"
	[ -d $(OBJFOLDER) ] && rm -r $(OBJFOLDER) || echo "no $(OBJFOLDER)"
	[ -f *.tikz ] && rm *.tikz || echo "no .tikz files"
	[ -f *.pid ] && rm *.pid || echo "no .pid files"
	[ -f *.log ] && rm *.log || echo "no .log files"
