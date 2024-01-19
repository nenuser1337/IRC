# Compiler settings - Can change to clang++ if required
CXX = g++

# Compiler flags for C++98
CXXFLAGS = -Wall -Wextra -std=c++98

# Build target executable
TARGET = server

# Object files
OBJS = ServerPoll.o main.o bot.o file_transfer.o

# First rule is the one executed when no parameters are fed to the Makefile
all: $(TARGET)

# Linking the executable from the object files
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# Compiling the source files into object files
ServerPoll.o: ServerPoll.cpp ServerPoll.hpp
	$(CXX) $(CXXFLAGS) -c ServerPoll.cpp

main.o: main.cpp ServerPoll.hpp
	$(CXX) $(CXXFLAGS) -c main.cpp

# Cleaning up the build
clean:
	rm -f $(TARGET) $(OBJS)

# This line is a customary Unix practice to tell make that these are not files
.PHONY: all clean
