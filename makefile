CXXFLAGS=-g -Wall
#CXXFLAGS+= -DDEBUG #uncomment if you want to display all debug messages
CXXFLAGS+= -std=c++11
LDFLAGS=
EXECUTABLE=nodable

TARGET :=linux64

BINDIR :=./bin/$(TARGET)
SRCDIR :=./sources
OBJDIR :=./build/$(TARGET)

VPATH = $(SRCDIR)

SOURCES := $(wildcard $(SRCDIR)/*.cpp)
OBJECTS := $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o,$(SOURCES))

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) -o $(BINDIR)/$(EXECUTABLE) $(OBJECTS) $(CFLAGS)

$(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	$(CXX) -c $(CXXFLAGS) ./$< -o ./$@

install:
	@echo "Starting install..."	
	@cp $(BINDIR)/$(EXECUTABLE) /bin && echo "Install done ! type nodable to launch the program." || echo "ERROR : Try with sudo make install" && exit
		
clean:
	rm -rf $(OBJECTS)

mrproper: clean
	rm -rf $(EXEC)
