LIBSRCS:=$(shell find src/lib -name '*.cpp')
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_SRC_FILES := $(LIBSRCS)
LOCAL_MODULE := jpcnn
LOCAL_C_INCLUDES += ./src/lib/include ./src/lib/graph ./src/lib/math ./src/lib/third_party ./src/lib/utility ./src/lib/svm ./src/lib/opengl ./src/lib ./src/include
include $(BUILD_SHARED_LIBRARY)
