CC=g++ -std=c++11
CFLAGS=-g -Wall
LDFLAGS=
EXEC=Node

all: $(EXEC)

Node: Node.o main.o
	$(CC) -o Node Node.o main.o $(LDFLAGS)

Node.o: Node.cpp
	$(CC) -o Node.o -c Node.cpp $(CFLAGS)

main.o: main.cpp Node.h
	$(CC) -o main.o -c main.cpp $(CFLAGS)

clean:
	rm -rf *.o

mrproper: clean
	rm -rf $(EXEC)
