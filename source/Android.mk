LIBSRCS:=$(shell find src/lib -path src/lib/pi -prune -o -name '*.cpp' -print)
${info ${LIBSRCS}}
NDK_PATH := /Users/zhengzhiheng/android-ndk-r9d/
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_SRC_FILES := $(LIBSRCS)
LOCAL_MODULE := jpcnn

LOCAL_C_INCLUDES += ./src/lib/include \
./src/lib/graph \
./src/lib/math \
./src/lib/third_party \
./src/lib/utility \
./src/lib/svm \
./src/lib/opengl \
./src/lib \
./src/include \
../eigen \
$(NDK_PATH)/sources/cxx-stl/gnu-libstdc++/4.6/include \
$(NDK_PATH)/sources/cxx-stl/gnu-libstdc++/4.6/libs/armeabi/include

LOCAL_CFLAGS := -DUSE_EIGEN_GEMM 
LOCAL_CFLAGS += -mfloat-abi=softfp -mfpu=neon -march=armv7

LOCAL_CFLAGS += -fopenmp -O3
LOCAL_LDFLAGS += -fopenmp -llog

include $(BUILD_SHARED_LIBRARY)
