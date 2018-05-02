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

all: makeDirs $(EXECUTABLE)

$(EXECUTABLE): $(OBJ)main.o $(OBJ)Node_Variable.o $(OBJ)Node.o $(OBJ)Node_Container.o $(OBJ)Log.o $(OBJ)Node_Value.o $(OBJ)Node_Number.o $(OBJ)Node_String.o $(OBJ)Node_Lexer.o
	$(CC) -o $(BIN)$(EXECUTABLE) $(OBJ)main.o $(OBJ)Node_Variable.o $(OBJ)Node_Container.o $(OBJ)Node.o $(OBJ)Log.o  $(OBJ)Node_Value.o $(OBJ)Node_Number.o $(OBJ)Node_String.o $(OBJ)Node_Lexer.o $(CPPFLAGS)

$(OBJ)Log.o: $(SRC)Log.cpp
	$(CC) -o $(OBJ)Log.o -c $(SRC)Log.cpp $(CPPFLAGS)

$(OBJ)Node.o: $(SRC)Node.cpp
	$(CC) -o $(OBJ)Node.o -c $(SRC)Node.cpp $(CPPFLAGS)
$(SRC)Node.cpp: $(SRC)Log.h $(SRC)Node_Number.h $(SRC)Node_String.h $(SRC)Node_Value.h $(SRC)Node_Container.h $(SRC)Node_Variable.h

$(OBJ)Node_Value.o: $(SRC)Node_Value.cpp
	$(CC) -o $(OBJ)Node_Value.o -c $(SRC)Node_Value.cpp $(CPPFLAGS)
$(SRC)Node_Value.cpp: $(SRC)Log.h $(SRC)Nodable.h $(SRC)Node_Value.h

$(OBJ)Node_Variable.o: $(SRC)Node_Variable.cpp
	$(CC) -o $(OBJ)Node_Variable.o -c $(SRC)Node_Variable.cpp $(CPPFLAGS)
$(SRC)Node_Variable.cpp: $(SRC)Log.h $(SRC)Node_Variable.h

$(OBJ)Node_Number.o: $(SRC)Node_Number.cpp
	$(CC) -o $(OBJ)Node_Number.o -c $(SRC)Node_Number.cpp $(CPPFLAGS)
$(SRC)Node_Number.cpp: $(SRC)Log.h $(SRC)Nodable.h $(SRC)Node_Value.h

$(OBJ)Node_String.o: $(SRC)Node_String.cpp
	$(CC) -o $(OBJ)Node_String.o -c $(SRC)Node_String.cpp $(CPPFLAGS)
$(SRC)Node_String.cpp: $(SRC)Log.h $(SRC)Nodable.h $(SRC)Node_Value.h

$(OBJ)Node_Container.o: $(SRC)Node_Container.cpp
	$(CC) -o $(OBJ)Node_Container.o -c $(SRC)Node_Container.cpp $(CPPFLAGS)
$(SRC)Node_Container.cpp: $(SRC)Log.h $(SRC)Node_Number.h $(SRC)Node_String.h $(SRC)Node_Variable.h

$(OBJ)Node_Lexer.o: $(SRC)Node_Lexer.cpp
	$(CC) -o $(OBJ)Node_Lexer.o -c $(SRC)Node_Lexer.cpp $(CPPFLAGS)
$(SRC)Node_Lexer.cpp: $(SRC)Log.h $(SRC)Node_Container.h $(SRC)Node.h $(SRC)Nodable.h $(SRC)Node_Value.h  $(SRC)Node_Number.h  $(SRC)Node_String.h

$(OBJ)main.o: $(SRC)main.cpp
	$(CC) -o $(OBJ)main.o -c $(SRC)main.cpp $(CPPFLAGS)
$(SRC)main.cpp: $(SRC)Log.h $(SRC)Nodable.h $(SRC)Node_String.h
	
cleanBuildAndRun:clean build run

build: all

run:
	$(BIN)$(EXECUTABLE)

makeDirs:
	mkdir -p $(OBJ)
	mkdir -p $(BIN)

clean:
	rm -rf $(OBJ)*.o

mrproper: clean
	rm -rf $(EXEC)
