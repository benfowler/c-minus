#######################################################################
# NAME:          Benjamin Fowler
# NUMBER:        02251132
# SUBJECT:       Modern Compiler Construction.
# INSTRUCTOR:    Dr Wayne Kelly
#######################################################################
# MODULE:        UNIX Makefile.
# DATE STARTED:  March 23, 2000.
# LAST EDITED:   March 23, 2000.
#######################################################################

CFLAGS = -g3 -W -Wall -ansi -pedantic 
CC = gcc
BIN = compiler
OBJ_FILES = Main.o Scan.o Parse.o Util.o SymTab.o Analyse.o CGen.o

$(BIN): $(OBJ_FILES) 
	$(CC) -g -o $(BIN) $(OBJ_FILES)

target: $(BIN)

#
# because the "clean" target isn't actually a file, we need to designate it
#  as "phony" so Make dosen't get confused.
#

.PHONY: clean

clean:
	rm -f core tags $(BIN) $(OBJ_FILES) *~ examples/*~ depends

ctags:
	ctags *

%.o: %.c
	$(CC) $(CFLAGS) -c $<

depends:
	gcc -MM *.c > depends

include depends

