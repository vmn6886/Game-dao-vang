# Compiler and flags
CXX       := g++
CXXFLAGS  := -std=c++23 -I src/include/SDL2 -g
LDFLAGS   := -L src/lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer

# Collect source files
SRCS      := $(wildcard *.cpp)
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
