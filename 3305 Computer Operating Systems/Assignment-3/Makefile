# Compiler and flags
CC = gcc
CFLAGS = -Wall -g

# Target executable
TARGET = asn3

# Build target
$(TARGET): assignment-3.o
	$(CC) $(CFLAGS) assignment-3.o -o $(TARGET)

# Compile .c to .o
assignment-3.o: assignment-3.c
	$(CC) $(CFLAGS) -c assignment-3.c


# Clean build files
PHONY: clean
clean:
	rm -f *.o $(TARGET)