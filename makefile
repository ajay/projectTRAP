CXX = g++
CFLAGS = `pkg-config --cflags opencv` -std=c++11 -O3 -g
LIBS = `pkg-config --libs opencv` -lchilitags

all: tags

tags: detect-live.cpp
	$(CXX) $(CFLAGS) -o $@ $< $(LIBS)

clean:
	rm -rfv tags
