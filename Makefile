CC=g++-5
INCDIR=-I. -I/home/matt/projects/external/github/folly/folly -I/home/matt/projects/external/github/readerwriterqueue
SRC=Main.c++ Tracking.c++ DataTypes.c++
LIB=-pthread
CFLAGS=-std=c++11 -O4

all:
	$(CC) $(CFLAGS) $(INCDIR) $(SRC) -o main $(LIB)
