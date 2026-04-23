# Compiler and flags
CC = gcc
CFLAGS = -Wall -g

# Target executable
TARGET = asn5

# Build target
$(TARGET): assignment-5.o
	$(CC) $(CFLAGS) assignment-5.o -o $(TARGET)

# Compile .c to .o
assignment-4.o: assignment-5.c
	$(CC) $(CFLAGS) -c assignment-5.c


# Clean build files
PHONY: clean
clean:
	rm -f *.o $(TARGET)