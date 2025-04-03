CXX = g++

CXXFLAGS = -std=c++17 -O2 -Wall `pkg-config --cflags opencv4`

LDFLAGS = `pkg-config --libs opencv4` -lpthread

TARGETS = headshots record_detect_faces

SRC_HEADSHOTS = headshots.cpp
SRC_RECORD_DETECT_FACES = record_detect_faces.cpp

all: $(TARGETS)

headshots: $(SRC_HEADSHOTS)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

record_detect_faces: $(SRC_RECORD_DETECT_FACES)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(TARGETS)


