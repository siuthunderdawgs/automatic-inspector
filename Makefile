SRC = $(shell find . -name '*.cpp')
OBJ = $(SRC:%.cpp=%.o)

BIN = automatic_inspector

# Get information for OpenCV
CFLAGS = `pkg-config --cflags opencv`
LFLAGS = `pkg-config --libs opencv`

# Get information for OpenCV2
# This should be automated at some point
CFLAGS += -I/usr/include/opencv2
LFLAGS +=

# Get information for libconfig++
CFLAGS += `pkg-config --cflags libconfig++`
LFLAGS += `pkg-config --libs libconfig++`

# Set compiler and linker flags
CFLAGS += -g -std=c++11
LFLAGS +=

$(BIN): $(OBJ)
	g++ -o $(BIN) $(OBJ) $(LFLAGS)

%.o: %.cpp
	g++ -c $< -o $@ $(CFLAGS)

clean:
	rm $(BIN) $(OBJ)
