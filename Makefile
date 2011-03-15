SOURCE_DIRS = .
SOURCES := $(subst ./,,$(wildcard $(SOURCE_DIRS:=/*.cpp)))
HEADERS := $(subst ./,,$(wildcard $(SOURCE_DIRS:=/*.h)))
OBJECTS := $(addprefix build/,$(SOURCES:.cpp=.o))
CXX = clang++
CXXFLAGS = -Wall -std=c++0x -gdwarf -I/usr/X11/include -I/usr/local/include/boost/ -fblocks
LDFLAGS = -L/usr/X11/lib -lpng 

all: build/pietc

build/pietc: $(OBJECTS)
	$(CXX) -o build/pietc $(OBJECTS) $(LDFLAGS)

build/%.o: %.cpp $(HEADERS) Makefile
	mkdir -p $(dir $@)
	$(CXX) -o $@ $(CXXFLAGS) -c $<

clean:
	rm -rf build

