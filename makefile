CC=g++
CPPFLAGS=-g -Wall
#CPPFLAGS+= -DDEBUG #uncomment if you want to display all debug messages
CPPFLAGS+= -std=c++11
LDFLAGS=
EXECUTABLE=Node.app

TARGET=linux64

SRC=./sources/
BIN=./bin/$(TARGET)/
OBJ=./build/$(TARGET)/

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJ)main.o $(OBJ)Node.o $(OBJ)Log.o   
	$(CC) -o $(BIN)$(EXECUTABLE) $(OBJ)main.o $(OBJ)Node.o $(OBJ)Log.o $(CPPFLAGS)

$(OBJ)Log.o: $(SRC)Log.cpp
	$(CC) -o $(OBJ)Log.o -c $(SRC)Log.cpp $(CPPFLAGS)

$(OBJ)Node.o: $(SRC)Node.cpp $(SRC)Log.h
	$(CC) -o $(OBJ)Node.o -c $(SRC)Node.cpp $(CPPFLAGS)

$(OBJ)main.o: $(SRC)main.cpp $(SRC)Log.h $(SRC)Nodable.h
	$(CC) -o $(OBJ)main.o -c $(SRC)main.cpp $(CPPFLAGS)

clean:
	rm -rf $(OBJ)*.o

mrproper: clean
	rm -rf $(EXEC)
