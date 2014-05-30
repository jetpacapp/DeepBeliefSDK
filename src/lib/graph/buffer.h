//
//  buffer.h
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#ifndef INCLUDE_BUFFER_H
#define INCLUDE_BUFFER_H

#include "jpcnn.h"
#include "dimensions.h"
#include "binary_format.h"
#include "offset.h"

class Buffer
{
public:

  Buffer(const Dimensions& dims);
  Buffer(const Dimensions& dims, jpfloat_t* data);
  Buffer(const Dimensions& dims, void* quantizedData, jpfloat_t min, jpfloat_t max, int bitsPerElement);
  Buffer(const Dimensions& dims, jpfloat_t min, jpfloat_t max, int bitsPerElement);
  virtual ~Buffer();

  Dimensions _dims;
  jpfloat_t* _data;
  void* _quantizedData;
#if defined(TARGET_PI)
  uint32_t _gpuMemoryHandle;
  uint32_t _gpuMemoryBase;
#endif // USE_QPU_GEMM
  jpfloat_t _min;
  jpfloat_t _max;
  int _bitsPerElement;
  bool _doesOwnData;
  char* _debugString;
  char* _name;

  // You must call canReshapeTo() to make sure the dimensions match before
  // calling reshape()
  bool canReshapeTo(const Dimensions& newDims);
  void reshape(const Dimensions& newDims);

  void copyDataFrom(const Buffer* other);
  void convertFromChannelMajor(const Dimensions& expectedDims);
  void populateWithRandomValues(jpfloat_t min, jpfloat_t max);
  void quantize(int bits);
  void transpose();

  // Creates a new buffer object that shares the underlying data array,
  // but has independent shape and other meta-data.
  Buffer* view();

  char* debugString();
  void printContents(int maxElements=8);
  void saveDebugImage();
  void setName(const char*);
};

extern Buffer* buffer_from_image_file(const char* filename);
extern Buffer* buffer_from_dump_file(const char* filename);
extern Buffer* buffer_from_tag_dict(SBinaryTag* mainDict, bool skipCopy);
extern void buffer_save_to_image_file(Buffer* buffer, const char* filename);
extern bool buffer_are_all_close(Buffer* a, Buffer* b, jpfloat_t tolerance=0.000001);
extern Buffer* buffer_view_at_top_index(Buffer* input, int index);
extern Buffer* convert_from_channeled_rgb_image(Buffer* input);
extern Buffer* convert_to_channeled_rgb_image(Buffer* input);
extern Buffer* extract_subregion(Buffer* input, const Offset& origin, const Dimensions& size);
SBinaryTag* buffer_to_tag_dict(Buffer* buffer, int floatBits = 32);
void buffer_dump_to_file(Buffer* buffer, const char* filename);
void quantize_buffer(Buffer* input, int howManyBits, jpfloat_t* outMin, jpfloat_t* outMax, void** outData, size_t* outSizeofData);

#endif // INCLUDE_BUFFER_H
