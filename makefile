ifeq ($(OS), Windows_NT)
DEFINE = -D _WINDOWS_
else
DEFINE = -D _LINUX_
endif

CC = gcc
CFLAGS = -Wall
BINPATH = ./bin
SRCPATH = ./src
INCPATH = ./src/include
INCLUDE = -I $(INCPATH)
OPTIONS = $(DEFINE) $(INCLUDE) $(CFLAGS)
#GENINC = $(INCPATH)/networking.h $(INCPATH)protocol.h
GENOBJ = $(BINPATH)/networking.o $(BINPATH)/protocol.o #$(BINPATH)/filetransfer.o


all: init mkdirs buildNyx buildXenia

mkdirs:
	mkdir -p bin
	mkdir -p bin/cfg

buildNyx: $(BINPATH)/server.o $(BINPATH)/Nyx.o $(GENOBJ)
	$(CC) $(OPTIONS) -o $(BINPATH)/Nyx $^

buildXenia: $(BINPATH)/client.o $(BINPATH)/Xenia.o $(GENOBJ)
	$(CC) $(OPTIONS) -o $(BINPATH)/Xenia $^

$(BINPATH)/%.o: $(SRCPATH)/%.c #$(INCPATH)/%.h $(GENINC)
	$(CC) $(OPTIONS) -c $< -o $@

clear:
	rm -f bin/*

init:
	rm -f -r bin

server srv Nyx nyx:
	./bin/Nyx

desrv:
	gdb ./bin/Nyx

client cli Xenia xen:
	./bin/Xenia

decli:
	gdb ./bin/Xenia

test:
	gcc test.c -o test

clt:
	rm test test.c