CC=g++ -std=c++11
CPPFLAGS=-g -Wall
CPPFLAGS += -DDEBUG
LDFLAGS=
OUTPUT=Node.app

TARGET=linux64

SRC=./sources/
BIN=./bin/$(TARGET)/
OBJ=./build/$(TARGET)/

all: $(OUTPUT)

$(OUTPUT): $(OBJ)Node.o $(OBJ)main.o
	$(CC) -o $(BIN)$(OUTPUT) $(OBJ)Node.o $(OBJ)main.o $(CPPFLAGS)

$(OBJ)Node.o: $(SRC)Node.cpp
	$(CC) -o $(OBJ)Node.o -c $(SRC)Node.cpp $(CPPFLAGS)

$(OBJ)main.o: $(SRC)main.cpp $(SRC)Node.h $(SRC)Nodable.h
	$(CC) -o $(OBJ)main.o -c $(SRC)main.cpp $(CPPFLAGS)

clean:
	rm -rf $(OBJ)*.o

mrproper: clean
	rm -rf $(EXEC)
