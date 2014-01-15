CC=gcc
CXX=g++
RM=rm -f
CPPFLAGS=-g -I ./src -I ./src/graph -I ./src/math -I ./src/third_party -I ./src/utility
LDFLAGS=-g -I foo
LDLIBS=

SRCS:=$(shell find $(SOURCEDIR) -name '*.cpp')
OBJS=$(subst .cpp,.o,$(SRCS))

all: jpcnn

jpcnn: $(OBJS)
	g++ $(LDFLAGS) -o jpcnn $(OBJS) $(LDLIBS) 

%.o: %.cpp
	$(CXX) $(CPPFLAGS) -c $< -o $(basename $@).o

clean:
	$(RM) $(OBJS)
