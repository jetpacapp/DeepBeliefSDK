CC=gcc
CXX=g++
RM=rm -f
CPPFLAGS=

LIBCPPFLAGS=-O3 -I ./src/lib/include -I ./src/lib/graph -I ./src/lib/math -I ./src/lib/third_party -I ./src/lib/utility -I ./src/lib/svm -I ./src/lib -I ./src/include
LIBLDFLAG=
LIBLDLIBS=

MKLROOT=/opt/intel/composer_xe_2013_sp1.0.080/mkl
LIBCPPFLAGS+= -fopenmp -DMKL_ILP64 -m64 -I$(MKLROOT)/include -DUSE_MKL_GEMM=1
LIBLDLIBS+= -Wl,--start-group /opt/intel/composer_xe_2013_sp1.0.080/mkl/lib/intel64/libmkl_intel_ilp64.a /opt/intel/composer_xe_2013_sp1.0.080/mkl/lib/intel64/libmkl_gnu_thread.a /opt/intel/composer_xe_2013_sp1.0.080/mkl/lib/intel64/libmkl_core.a -Wl,--end-group -ldl -lpthread -lm
LIBLDLIBS+= -L/usr/lib/gcc/x86_64-linux-gnu/4.6/ -lgomp

LIBSRCS:=$(shell find src/lib -name '*.cpp')
LIBOBJS=$(subst .cpp,.o,$(LIBSRCS))

TOOLCPPFLAGS=-O3 -I ./src/include

TOOLSRCS:=$(shell find src/tool -name '*.cpp')
TOOLOBJS=$(subst .cpp,.o,$(TOOLSRCS))

all: jpcnn

libjpcnn.so: CPPFLAGS=$(LIBCPPFLAGS)
libjpcnn.so: $(LIBOBJS)
	g++ -shared $(LIBLDFLAGS) -o libjpcnn.so $(LIBOBJS) $(LIBLDLIBS) 

main.o: src/tool/main.cpp
	$(CXX) $(CPPFLAGS) -c src/tool/main.cpp -o main.o

jpcnn: CPPFLAGS=$(TOOLCPPFLAGS)
jpcnn: libjpcnn.so $(TOOLOBJS)
	g++ -o jpcnn $(TOOLOBJS) -L. -ljpcnn

%.o: %.cpp
	$(CXX) $(CPPFLAGS) -fPIC -c $< -o $(basename $@).o

clean:
	find . -iname "*.o" -exec rm '{}' ';'
