# Compiler and Standard
CXX = g++
CXXFLAGS = -std=c++17 -Wall `wx-config --cxxflags`

# Linker flags
LDFLAGS = `wx-config --libs`

# Target executable name
TARGET = FileManager

# Source files
SRCS = Main.cpp FileSystem.cpp FileManagerFrame.cpp

# Object files (automatically replaces .cpp with .o)
OBJS = $(SRCS:.cpp=.o)

# Default rule
all: $(TARGET)

# Link the executable
$(TARGET): $(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LDFLAGS)

# Compile source files into object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -f $(OBJS) $(TARGET)

# Phony targets
.PHONY: all clean
