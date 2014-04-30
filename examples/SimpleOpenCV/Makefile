CC=gcc
CXX=g++
RM=rm -f

CPPFLAGS := -O3 -I .

SRCS := $(shell find . -name '*.cpp')
OBJS := $(subst .cpp,.o,$(SRCS))

OPENCV_DIR := /home/vagrant/projects/opencv

all: deepopencv

deepopencv: $(OBJS)
	$(CXX) -o deepcv $(OBJS) \
-L/usr/lib \
-L/usr/local/lib \
-L$(OPENCV_DIR)/build/3rdparty/lib \
-ljpcnn \
-lopencv_objdetect \
-lopencv_highgui \
-lopencv_imgproc \
-lopencv_core \
-lIlmImf \
-llibtiff \
-lpng \
-llibjpeg \
-llibjasper \
-llibwebp \
-lpthread \
-lrt

%.o: %.cpp
	$(CXX) $(CPPFLAGS) -fPIC -c $< -o $(basename $@).o

clean:
	find . -iname "*.o" -exec rm '{}' ';'

