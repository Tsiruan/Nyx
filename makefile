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
GENOBJ = networking.o protocol.o


all: init mkdirs buildNyx buildXenia

mkdirs:
	mkdir -p bin
	mkdir -p bin/cfg

# build server and client
buildNyx: server.o Nyx.o $(GENOBJ)
	$(CC) $(OPTIONS) -o $(BINPATH)/Nyx $(patsubst %.o, $(BINPATH)/%.o, $^)

buildXenia: client.o Xenia.o $(GENOBJ)
	$(CC) $(OPTIONS) -o $(BINPATH)/Xenia $(patsubst %.o, $(BINPATH)/%.o, $^)

# compile all object files
%.o: $(SRCPATH)/%.c
	$(CC) $(OPTIONS) -c $< -o $(BINPATH)/$@

# delete all binaries
init:
	rm -f -r bin

# run server or client
server srv Nyx nyx:
	./bin/Nyx

client cli Xenia xen:
	./bin/Xenia



ds:
	gdb ./bin/Nyx

dc:
	gdb ./bin/Xenia

test:
	gcc test.c -o test

clt:
	rm test test.c