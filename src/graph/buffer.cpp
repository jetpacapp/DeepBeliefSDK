//
//  bbuffer.cpp
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#include "buffer.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "stb_image.h"
#include "binary_format.h"

static void buffer_do_save_to_image_file(Buffer* buffer, const char* filename);

Buffer::Buffer(const Dimensions& dims) : _dims(dims), _name(NULL), _debugString(NULL)
{
  const int elementCount = _dims.elementCount();
  const size_t byteCount = (elementCount * sizeof(jpfloat_t));
  _data = (jpfloat_t*)(malloc(byteCount));
  _doesOwnData = true;
  setName("None");
}

Buffer::Buffer(const Dimensions& dims, jpfloat_t* data) : _dims(dims), _name(NULL), _debugString(NULL) {
  _data = data;
  _doesOwnData = false;
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

void Buffer::setName(const char* name) {
  if (_name)
  {
    free(_name);
  }
  size_t byteCount = (strlen(name) + 1);
  _name = (char*)(malloc(byteCount));
  strncpy(_name, name, byteCount);
}

void Buffer::saveDebugImage() {
  buffer_save_to_image_file(this, this->_name);
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

  SBinaryTag* mainDict = read_tag_from_file(inputFile);
  assert(mainDict != NULL);

  Buffer* result = buffer_from_tag_dict(mainDict);
  result->setName(filename);

  fclose(inputFile);
  free(mainDict);

  return result;
}

Buffer* buffer_from_tag_dict(SBinaryTag* mainDict) {

  SBinaryTag* bitsPerFloatTag = get_tag_from_dict(mainDict, "float_bits");
  const uint32_t bitsPerFloat = bitsPerFloatTag->payload.jpuint;
  if (bitsPerFloat != 32) {
    fprintf(stderr, "jpcnn can only read 32-bit float dump files, found %d\n", bitsPerFloat);
    return NULL;
  }

  SBinaryTag* dimsTag = get_tag_from_dict(mainDict, "dims");
  assert(dimsTag->type == JP_LIST);
  int32_t dimensions[DIMENSIONS_MAX_LENGTH];
  int32_t dimensionsCount = 0;
  char* current = dimsTag->payload.jpchar;
  char* end = (current + dimsTag->length);
  while (current < end) {
    SBinaryTag* entryTag = get_tag_from_memory(current, end);
    current += get_total_sizeof_tag(entryTag);
    assert(entryTag->type == JP_UINT);
    dimensions[dimensionsCount] = entryTag->payload.jpuint;
    dimensionsCount += 1;
  }

  Dimensions dims(dimensions, dimensionsCount);
  Buffer* buffer = new Buffer(dims);

  SBinaryTag* dataTag = get_tag_from_dict(mainDict, "data");
  assert(dataTag->type == JP_FARY);

  const int elementCount = buffer->_dims.elementCount();
  assert(dataTag->length == (elementCount * sizeof(jpfloat_t)));
  memcpy(buffer->_data, dataTag->payload.jpchar, dataTag->length);

  return buffer;
}

void buffer_save_to_image_file(Buffer* buffer, const char* basename) {
  const int maxFilenameLength = 1024;
  char filename[maxFilenameLength];

  const Dimensions dims = buffer->_dims;
  assert((dims._length == 4) || (dims._length == 3));

  if (dims._length == 3) {
    snprintf(filename, maxFilenameLength, "%s.ppm", basename);
    buffer_do_save_to_image_file(buffer, filename);
  } else {
    for (int index = 0; index < dims[0]; index += 1) {
      Buffer* view = buffer_view_at_top_index(buffer, index);
      snprintf(filename, maxFilenameLength, "%s_%02d.ppm", basename, index);
      buffer_do_save_to_image_file(view, filename);
    }
  }
}

void buffer_do_save_to_image_file(Buffer* buffer, const char* filename) {

  const Dimensions dims = buffer->_dims;
  assert(dims._length == 3);

  const int width = dims[1];
  const int height = dims[0];
  const int channels = MIN(dims[2], 3);

  const jpfloat_t* const data = buffer->_data;

  FILE* outputFile = fopen(filename, "w");
  if (!outputFile) {
    fprintf(stderr, "jpcnn couldn't open '%s' for writing\n", filename);
    return;
  }

  fprintf(outputFile, "P3\n");
  fprintf(outputFile, "# CREATOR: jpcnn from %s\n", buffer->debugString());

  fprintf(outputFile, "%d %d\n", width, height);
  fprintf(outputFile, "255\n");

  for (int y = 0; y < height; y += 1) {
    for (int x = 0; x < width; x += 1) {
      for (int channel = 0; channel < 3; channel += 1) {
        int value;
        if (channel >= channels) {
          value = 0;
        } else {
          value = (int)(*(data + dims.offset(y, x, channel)));
        }
        fprintf(outputFile, "%d ", value);
      }
    }
  }
  fclose(outputFile);
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
  jpfloat_t totalDelta = 0.0f;
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
    totalDelta += absDelta;
    aCurrent += 1;
    bCurrent += 1;
  }

  if (differentCount > 0) {
    const jpfloat_t differentPercentage = 100 * (differentCount / (jpfloat_t)(a->_dims.elementCount()));
    const jpfloat_t meanDelta = (totalDelta / a->_dims.elementCount());
    fprintf(stderr, "Buffers contained %f%% different values (%d), mean delta = %f - %s vs %s\n",
      differentPercentage,
      differentCount,
      meanDelta,
      a->debugString(),
      b->debugString());
    return false;
  }

  return true;
}

Buffer* buffer_view_at_top_index(Buffer* input, int index) {
  const Dimensions inputDims = input->_dims;
  assert(inputDims._length > 1);
  assert(index < inputDims[0]);
  Dimensions outputDims = inputDims.removeDimensions(1);
  const int topStride = outputDims.elementCount();
  jpfloat_t* const viewData = (input->_data + (topStride * index));
  Buffer* output = new Buffer(outputDims, viewData);
  return output;
}

