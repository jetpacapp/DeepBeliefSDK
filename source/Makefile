CC=gcc
CXX=g++
RM=rm -f
CPPFLAGS=

.PRECIOUS: %.cdat

LIBCPPFLAGS=-Ofast -I ./src/lib/include -I ./src/lib/graph -I ./src/lib/math -I ./src/lib/third_party -I ./src/lib/utility -I ./src/lib/svm -I ./src/lib/opengl -I ./src/lib -I ./src/include -g
LIBLDFLAG=
LIBLDLIBS=

$(warning GEMM=$(GEMM))
$(warning TARGET=$(TARGET))

LIBSRCS := $(shell find src/lib -name '*.cpp' -not -name '._*')
LIBOBJS := $(subst .cpp,.o,$(LIBSRCS))

ifeq ($(GEMM),mkl)
MKLROOT = /opt/intel/composer_xe_2013_sp1.0.080/mkl
LIBCPPFLAGS += -fopenmp -DMKL_ILP64 -m64 -I$(MKLROOT)/include -DUSE_MKL_GEMM=1
LIBLDLIBS += -Wl,--start-group /opt/intel/composer_xe_2013_sp1.0.080/mkl/lib/intel64/libmkl_intel_ilp64.a /opt/intel/composer_xe_2013_sp1.0.080/mkl/lib/intel64/libmkl_gnu_thread.a /opt/intel/composer_xe_2013_sp1.0.080/mkl/lib/intel64/libmkl_core.a -Wl,--end-group -ldl -lpthread -lm
LIBLDLIBS += -L/usr/lib/gcc/x86_64-linux-gnu/4.6/ -lgomp
endif

ifeq ($(GEMM),atlas)
LIBCPPFLAGS += -I/usr/include -DUSE_ATLAS_GEMM=1
LIBLDLIBS += -lblas
endif

ifeq ($(GEMM),eigen)
LIBCPPFLAGS += -I../eigen -DUSE_EIGEN_GEMM=1 
endif

ifeq ($(TARGET),pi)
LIBCPPFLAGS += \
-DTARGET_PI \
-march=armv6 \
-mfloat-abi=hard \
-ftree-vectorize \
-funroll-all-loops \
-mfpu=vfp \
-I./src/lib/pi
endif

ifeq ($(TARGET),pi2)
LIBCPPFLAGS += \
-mcpu=cortex-a7 \
-mfpu=neon-vfpv4 \
-mfloat-abi=hard
endif

ifeq ($(TARGET),beagle)
LIBCPPFLAGS += \
-march=armv7-a \
-mtune=cortex-a8 \
-mfloat-abi=hard \
-mfpu=neon \
-ffast-math
endif

ifeq ($(GEMM),pigl)
LIBCPPFLAGS += \
-I/usr/include \
-I/opt/vc/include/ \
-I/opt/vc/include/interface/vcos/pthreads/ \
-I/opt/vc/include/interface/vmcs_host/linux/ \
-DUSE_GL_GEMM \
-DUSE_OPENGL \
-DTARGET_PI \
-DDEBUG
LIBLDLIBS += -lblas -L/opt/vc/lib -lGLESv2 -lEGL -lopenmaxil -lbcm_host
endif

ifeq ($(GEMM),piqpu)
LIBCPPFLAGS += -DUSE_QPU_GEMM -g
ASMSRCS := ./src/lib/pi/gemm_float.asm ./src/lib/pi/gemm_8bit.asm ./src/lib/pi/gemm_16bit.asm
ASMINTERMEDIATES := $(subst .asm,.cdat,$(ASMSRCS))
ASMOBJS := $(subst .asm,.do,$(ASMSRCS))
LIBOBJS += $(ASMOBJS)
endif

TOOLCPPFLAGS := -O3 -I ./src/include -g

TOOLSRCS := $(shell find src/tool -name '*.cpp' -not -name '._*')
TOOLOBJS := $(subst .cpp,.o,$(TOOLSRCS))

all: jpcnn

%.cdat: %.asm
	m4 -I ./src/lib/pi/ $< | qpu-asm -o $(basename $@).cdat -c g_$(notdir $(basename $@))Code

%.do: %.cdat
	$(CXX) $(CPPFLAGS) -x c -c $< -o $(basename $@).do	

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
	find . -iname "*.do" -exec rm '{}' ';'
