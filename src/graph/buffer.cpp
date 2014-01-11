//
//  bbuffer.cpp
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#include "buffer.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "math.h"

#include "stb_image.h"

Buffer::Buffer(const Dimensions& dims) : _dims(dims)
{
  const int elementCount = _dims.elementCount();
  const size_t byteCount = (elementCount * sizeof(jpfloat_t));
  _data = (jpfloat_t*)(malloc(byteCount));
  _debugString = NULL;
  setName("None");
}

Buffer::~Buffer()
{
  free(_data);
  if (_debugString)
  {
    free(_debugString);
  }
  if (_name)
  {
    free(_name);
  }
}

char* Buffer::debugString()
{
  if (!_debugString)
  {
    _debugString = (char*)(malloc(MAX_DEBUG_STRING_LEN));
  }
  snprintf(_debugString, MAX_DEBUG_STRING_LEN, "Buffer %s - %s", _name, _dims.debugString());
  return _debugString;
}

void Buffer::setName(const char* name)
{
  if (_name)
  {
    free(_name);
  }
  size_t byteCount = (strlen(name) + 1);
  _name = (char*)(malloc(byteCount));
  strncpy(_name, name, byteCount);
}

Buffer* buffer_from_image_file(const char* filename)
{
  FILE* inputFile = fopen(filename, "rb");
  if (!inputFile) {
    fprintf(stderr, "jpcnn couldn't open '%s'\n", filename);
    return NULL;
  }

  int inputWidth;
  int inputHeight;
  int inputChannels;
  uint8_t* imageData = stbi_load_from_file(inputFile, &inputWidth, &inputHeight, &inputChannels, 0);
  fclose(inputFile);

  Dimensions dims(inputHeight, inputWidth, inputChannels);
  Buffer* buffer = new Buffer(dims);
  buffer->setName(filename);

  jpfloat_t* bufferCurrent = buffer->_data;
  jpfloat_t* bufferEnd = buffer->dataEnd();
  uint8_t* imageCurrent = imageData;
  while (bufferCurrent != bufferEnd)
  {
    *bufferCurrent = *imageCurrent;
    bufferCurrent += 1;
    imageCurrent += 1;
  }

  stbi_image_free(imageData);

  return buffer;
}

Buffer* buffer_from_dump_file(const char* filename)
{
  FILE* inputFile = fopen(filename, "rb");
  if (!inputFile) {
    fprintf(stderr, "jpcnn couldn't open '%s'\n", filename);
    return NULL;
  }

  uint32_t bitsPerFloat;
  fread(&bitsPerFloat, sizeof(bitsPerFloat), 1, inputFile);
  if (bitsPerFloat != 32) {
    fprintf(stderr, "jpcnn can only read 32-bit float dump files, found %d for '%s'\n", bitsPerFloat, filename);
    return NULL;
  }

  int32_t dimensionsCount;
  fread(&dimensionsCount, sizeof(dimensionsCount), 1, inputFile);
  if (dimensionsCount > DIMENSIONS_MAX_LENGTH) {
    fprintf(stderr, "jpcnn can only read %d dimensional dump files, found %d for '%s'\n", DIMENSIONS_MAX_LENGTH, dimensionsCount, filename);
    return NULL;
  }

  int32_t dimensions[DIMENSIONS_MAX_LENGTH];
  fread(dimensions, sizeof(int32_t), dimensionsCount, inputFile);

  Dimensions dims(dimensions, dimensionsCount);
  Buffer* buffer = new Buffer(dims);
  buffer->setName(filename);

  const int elementCount = buffer->_dims.elementCount();
  const size_t elementsRead = fread(buffer->_data, sizeof(jpfloat_t), elementCount, inputFile);
  if (elementsRead != elementCount) {
    fprintf(stderr, "jpcnn expected %d elements, found %zu for '%s'\n", elementCount, elementsRead, filename);
    return NULL;
  }

  fclose(inputFile);

  return buffer;
}

bool buffer_are_all_close(Buffer* a, Buffer* b, jpfloat_t tolerance) {

  if (a == NULL) {
    fprintf(stderr, "Buffer a is NULL\n");
    return false;
  }

  if (b == NULL) {
    fprintf(stderr, "Buffer b is NULL\n");
    return false;
  }

  if (a->_dims._length != b->_dims._length) {
    fprintf(stderr, "Buffers have different numbers of dimensions - %s vs %s\n", a->debugString(), b->debugString());
    return false;
  }

  for (int index = 0; index < a->_dims._length; index += 1) {
    if (a->_dims._dims[index] != b->_dims._dims[index]) {
      fprintf(stderr, "Buffers are different sizes - %s vs %s\n", a->debugString(), b->debugString());
      return false;
    }
  }

  int differentCount = 0;
  jpfloat_t* aCurrent = a->_data;
  jpfloat_t* aEnd = a->dataEnd();
  jpfloat_t* bCurrent = b->_data;
  while (aCurrent != aEnd) {
    const jpfloat_t aValue = *aCurrent;
    const jpfloat_t bValue = *bCurrent;
    const jpfloat_t delta = (aValue - bValue);
    const jpfloat_t absDelta = fabsf(delta);
    if (absDelta > tolerance) {
      differentCount += 1;
    }
    aCurrent += 1;
    bCurrent += 1;
  }

  if (differentCount > 0) {
    const jpfloat_t differentPercentage = (differentCount / (jpfloat_t)(a->_dims.elementCount()));
    fprintf(stderr, "Buffers contained %f%% different values - %s vs %s\n", differentPercentage, a->debugString(), b->debugString());
    return false;
  }

  return true;
}
