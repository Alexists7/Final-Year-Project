CXX = g++
CXXFLAGS = -Wall -Wextra -pedantic -std=c++11 `pkg-config --cflags opencv4`
LDFLAGS = `pkg-config --libs opencv4`

TARGETS = motion_detector 
SRCS = motion_detector.cpp 

all: $(TARGETS)

motion_detector: motion_detector.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(TARGETS)
