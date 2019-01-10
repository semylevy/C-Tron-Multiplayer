# Simple example of a Makefile

### Variables for this project ###
# These should be the only ones that need to be modified
# The files that must be compiled, with a .o extension
OBJECTS = fatal_error.o sockets.o tron_simulation.o
# The header files
DEPENDS = fatal_error.h sockets.h codes.h tron_simulation.h
# The executable programs to be created
CLIENT = client
SERVER = server

### Variables for the compilation rules ###
# These should work for most projects, but can be modified when necessary
# The compiler program to use
CC = gcc
# Options to use when compiling object files
# NOTE the use of gnu99, because otherwise the socket structures are not included
#  http://stackoverflow.com/questions/12024703/why-cant-getaddrinfo-be-found-when-compiling-with-gcc-and-std-c99
CFLAGS = -Wall -g -std=gnu99 -pedantic # -O2
# Options to use for the final linking process
# This one links the math library
LDLIBS = -lpthread -lcsfml-window -lcsfml-graphics -lcsfml-system
LDFLAGS=-lncurses

### The rules ###
# These should work for most projects without change
# Special variable names:
#   $@  = The name of the rule
#   $^  = All the requirements for the rule
#   $<  = The first required file of the rule

# Default rule
all: $(CLIENT) $(SERVER) $(TEST)

# Rule to make the client program
$(CLIENT): $(CLIENT).o $(OBJECTS)
	$(CC) $^ -o $@ $(LDFLAGS) $(LDLIBS)

# Rule to make the server program
$(SERVER): $(SERVER).o $(OBJECTS)
	$(CC) $^ -o $@ $(LDFLAGS) $(LDLIBS)

# Rule to make the server program
$(TEST): $(TEST).o $(OBJECTS)
	$(CC) $^ -o $@ $(LDFLAGS) $(LDLIBS)

# Rule to make the object files
%.o: %.c $(DEPENDS)
	$(CC) $< -c -o $@ $(CFLAGS)

# Clear the compiled files
clean:
	rm -rf *.o $(CLIENT) $(SERVER) $(TEST)
	
# Indicate the rules that do not refer to a file
.PHONY: clean all