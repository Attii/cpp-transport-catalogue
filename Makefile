# Compiler and compiler flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g

# App name
APP_NAME = app

# Object files
OBJS = domain.o geo.o json.o json_reader.o main.o map_renderer.o request_handler.o svg.o transport_catalogue.o

# Header files
DEPS = domain.h geo.h json.h json_reader.h map_renderer.h request_handler.h svg.h transport_catalogue.h

# Default target
all: $(APP_NAME)

# Linking
$(APP_NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compiling
%.o: %.cpp $(DEPS)
	$(CXX) $(CXXFLAGS) -c $<

# Clean up
clean:
	rm -f $(OBJS) $(APP_NAME)
