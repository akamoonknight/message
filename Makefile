CC=g++-5
INCDIR=-I./include -I/home/matt/projects/external/github/folly/folly -I/home/matt/projects/external/github/readerwriterqueue
SRC=src/Main.c++ src/Tracking.c++ src/DataTypes.c++
BIN=bin/main
LIB=-pthread
CFLAGS=-std=c++11 -O4

all:
	$(CC) $(CFLAGS) $(INCDIR) $(SRC) -o $(BIN) $(LIB)
