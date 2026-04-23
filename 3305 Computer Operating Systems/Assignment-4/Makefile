# Compiler and flags
CC = gcc
CFLAGS = -Wall -g

# Target executable
TARGET = asn4

# Build target
$(TARGET): assignment-4.o
	$(CC) $(CFLAGS) assignment-4.o -o $(TARGET)

# Compile .c to .o
assignment-4.o: assignment-4.c
	$(CC) $(CFLAGS) -c assignment-4.c


# Clean build files
PHONY: clean
clean:
	rm -f *.o $(TARGET)