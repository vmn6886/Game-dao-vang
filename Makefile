# Compiler and flags
CXX       := g++
CXXFLAGS  := -I src/include/SDL2 -g
LDFLAGS   := -L src/lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer

# Collect all .cpp files (update path if necessary)
# SRCS      := $(wildcard *.cpp)
SRCS 		:= background.cpp
# If your source files are in a subdirectory (e.g., src/), use:
# SRCS := $(wildcard src/*.cpp)

# Convert source files to object files
OBJS      := $(SRCS:.cpp=.o)

# Name of the output executable
TARGET    := main

# Default target
all: $(TARGET)

# Link object files to create the executable and then remove the .o files
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)
	rm -f $(OBJS)

# Compile .cpp files into .o files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
