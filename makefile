ifeq ($(OS), Windows_NT)
DEFINE = -D _WINDOWS_
else
DEFINE = -D _LINUX_
endif

BINPATH = ./bin
SRCPATH = ./src
NYXPATH = Nyx
XENPATH = Xenia
INCPATH = ./src/include

# put all names of source file here, and it will compile!
NYXSRC = server.c Nyx.c onlineTable.c database.c automata.c
XENSRC = client.c Xenia.c automata.c
GENSRC = networking.c protocol.c utility.c
NYXOBJ = $(patsubst %.c, $(NYXPATH)/%.o, $(NYXSRC))
XENOBJ = $(patsubst %.c, $(XENPATH)/%.o, $(XENSRC))
GENOBJ = $(patsubst %.c, %.o, $(GENSRC))

CC = gcc
CFLAGS = -Wall
INCLUDE = -I $(INCPATH)
OPTIONS = $(DEFINE) $(INCLUDE) $(CFLAGS)


all: init mkdirs buildNyx buildXenia

# delete all binaries and its' folders
# .o files doesn't have dependencies on headers, thus recompile everytime for now
init:
	rm -f -r bin

mkdirs:
	mkdir -p bin
	mkdir -p bin/cfg

# build server and client
buildNyx: $(NYXOBJ) $(GENOBJ)
	$(CC) $(OPTIONS) -o $(BINPATH)/Nyx $(patsubst %.o, $(BINPATH)/%.o, $(notdir $^))

buildXenia: $(XENOBJ) $(GENOBJ)
	$(CC) $(OPTIONS) -o $(BINPATH)/Xenia $(patsubst %.o, $(BINPATH)/%.o, $(notdir $^))

# compile all object files
# not a neat coding, fix it afterwards
$(NYXPATH)/%.o: $(SRCPATH)/$(NYXPATH)/%.c
	$(CC) $(OPTIONS) -c $< -o $(BINPATH)/$(notdir $@)

$(XENPATH)/%.o: $(SRCPATH)/$(XENPATH)/%.c
	$(CC) $(OPTIONS) -c $< -o $(BINPATH)/$(notdir $@)

%.o: $(SRCPATH)/%.c
	$(CC) $(OPTIONS) -c $< -o $(BINPATH)/$(notdir $@)

# run server or client
server srv Nyx nyx:
	./bin/Nyx

client cli Xenia xen:
	./bin/Xenia