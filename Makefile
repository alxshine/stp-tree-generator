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

all: $(OBJFOLDER)/main.o $(OBJFOLDER)/Mac.o $(OBJFOLDER)/Bridge.o $(OBJFOLDER)/Sniffer.o $(OBJFOLDER)/SpanningTree.o
	[ -d $(BINFOLDER) ] || mkdir $(BINFOLDER)
	$(CC) $(CFLAGS) $^ $(INCLUDES) -o $(BINFOLDER)/stpGen

clean:
	rm -r $(BINFOLDER) $(OBJFOLDER)
