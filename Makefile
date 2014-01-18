CC=gcc
CXX=g++
RM=rm -f
CPPFLAGS=-O3 -I ./src -I ./src/graph -I ./src/math -I ./src/third_party -I ./src/utility
LDFLAG=
LDLIBS=

MKLROOT=/opt/intel/composer_xe_2013_sp1.0.080/mkl
CPPFLAGS+= -fopenmp -DMKL_ILP64 -m64 -I$(MKLROOT)/include -DUSE_MKL_GEMM=1
LDLIBS+= -Wl,--start-group /opt/intel/composer_xe_2013_sp1.0.080/mkl/lib/intel64/libmkl_intel_ilp64.a /opt/intel/composer_xe_2013_sp1.0.080/mkl/lib/intel64/libmkl_gnu_thread.a /opt/intel/composer_xe_2013_sp1.0.080/mkl/lib/intel64/libmkl_core.a -Wl,--end-group -ldl -lpthread -lm
LDLIBS+= -L/usr/lib/gcc/x86_64-linux-gnu/4.6/ -lgomp

SRCS:=$(shell find $(SOURCEDIR) -name '*.cpp')
LIBOBJS=$(subst .cpp,.o,$(SRCS))

all: jpcnn

libjpcnn.so: $(LIBOBJS)
	g++ -shared $(LDFLAGS) -o libjpcnn.so $(LIBOBJS) $(LDLIBS) 

main.o: src/main.cpp
	$(CXX) $(CPPFLAGS) -c src/main.cpp -o main.o

jpcnn: libjpcnn.so main.o
	g++ -o jpcnn -L. -ljpcnn main.o

%.o: %.cpp
	$(CXX) $(CPPFLAGS) -fPIC -c $< -o $(basename $@).o

clean:
	find . -iname "*.o" -exec rm '{}' ';'
