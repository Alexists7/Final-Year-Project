# Define the compiler
CXX = g++

# Define the compiler flags
CXXFLAGS = -std=c++17 -O2 -Wall `pkg-config --cflags opencv4` `pkg-config --cflags dlib-1`

# Define the linker flags
LDFLAGS = `pkg-config --libs opencv4` `pkg-config --libs dlib-1`

# Define the target executables
TARGETS = headshots record_detect_faces 

# Define the source files
SRC_HEADSHOTS = headshots.cpp 
SRC_RECORD_DETECT_FACES = record_detect_faces.cpp

all: $(TARGETS)

# Rule for building the headshots executable
headshots: $(SRC_HEADSHOTS)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

# Rule for building the record_detect_faces executable
record_detect_faces: $(SRC_RECORD_DETECT_FACES)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)	

# Clean rule to remove the executables
clean:
	rm -f $(TARGETS)


