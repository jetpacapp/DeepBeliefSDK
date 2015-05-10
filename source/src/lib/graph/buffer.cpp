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
#include <float.h>

#ifdef USE_ACCELERATE_GEMM
#include <Accelerate/Accelerate.h>
#endif

#ifdef USE_OS_IMAGE
#include "os_image_load.h"
#include "os_image_save.h"
#else // USE_OS_IMAGE
#include "stb_image.h"
#endif // USE_OS_IMAGE
#include "binary_format.h"
#include "cstring_helpers.h"
#ifdef TARGET_PI
#include "mailbox.h"
#endif // TARGET_PI

#ifdef USE_EIGEN_GEMM
#include <Eigen/Core>
#endif // USE_EIGEN_GEMM

static void buffer_do_save_to_image_file(Buffer* buffer, const char* filename);

Buffer::Buffer(const Dimensions& dims) :
  _dims(dims),
  _name(NULL),
  _debugString(NULL),
  _quantizedData(NULL),
  _min(0.0f),
  _max(1.0f),
  _bitsPerElement(32)
{
  const int elementCount = _dims.elementCount();
  const size_t byteCount = (elementCount * sizeof(jpfloat_t));
#if defined(TARGET_PI)
  _gpuMemoryHandle = mem_alloc(byteCount, 4096, GPU_MEM_FLG);
  if (!_gpuMemoryHandle) {
    fprintf(stderr, "Unable to allocate %d bytes of GPU memory\n", byteCount);
  }
  _gpuMemoryBase = mem_lock(_gpuMemoryHandle);
  _data = (jpfloat_t*)(mapmem(_gpuMemoryBase + GPU_MEM_MAP, byteCount));
#elif defined(USE_EIGEN_GEMM)
  _data = (jpfloat_t*)(Eigen::internal::aligned_malloc(byteCount));
#else // TARGET_PI
  _data = (jpfloat_t*)(malloc(byteCount * 1));
#endif // TARGET_PI
  _doesOwnData = true;
  setName("None");
}

Buffer::Buffer(const Dimensions& dims, jpfloat_t* data) :
  _dims(dims),
  _name(NULL),
  _debugString(NULL),
  _quantizedData(NULL),
#if defined(TARGET_PI)
  _gpuMemoryHandle(0),
  _gpuMemoryBase(0),
#endif // TARGET_PI
  _min(0.0f),
  _max(1.0f),
  _bitsPerElement(32)
{
  _data = data;
  _doesOwnData = false;
  setName("None");
}

Buffer::Buffer(const Dimensions& dims, void* quantizedData, jpfloat_t min, jpfloat_t max, int bitsPerElement) :
  _dims(dims),
  _name(NULL),
  _debugString(NULL),
  _data(NULL),
#if defined(TARGET_PI)
  _gpuMemoryHandle(0),
  _gpuMemoryBase(0),
#endif // TARGET_PI
  _min(min),
  _max(max),
  _bitsPerElement(bitsPerElement)
{
  _quantizedData = quantizedData;
  _doesOwnData = false;
  setName("None");
}

Buffer::Buffer(const Dimensions& dims, jpfloat_t min, jpfloat_t max, int bitsPerElement) :
  _dims(dims),
  _name(NULL),
  _debugString(NULL),
  _data(NULL),
  _min(min),
  _max(max),
  _bitsPerElement(bitsPerElement)
{
  const int elementCount = _dims.elementCount();
  const int sizeofElement = (_bitsPerElement / 8);
  const size_t byteCount = (elementCount * sizeofElement);

#if defined(TARGET_PI)
  _gpuMemoryHandle = mem_alloc(byteCount, 4096, GPU_MEM_FLG);
  if (!_gpuMemoryHandle) {
    fprintf(stderr, "Unable to allocate %d bytes of GPU memory\n", byteCount);
  }
  _gpuMemoryBase = mem_lock(_gpuMemoryHandle);
  _quantizedData = (char*)(mapmem(_gpuMemoryBase + GPU_MEM_MAP, byteCount));
#elif defined(USE_EIGEN_GEMM)
  _quantizedData = (void*)(Eigen::internal::aligned_malloc(byteCount));
#else // TARGET_PI
  _quantizedData = (void*)(malloc(byteCount));
#endif // TARGET_PI
  _doesOwnData = true;
  setName("None");
}

Buffer::~Buffer()
{
  if (_doesOwnData) {
#if defined(TARGET_PI)
    if (_data) {
      const int elementCount = _dims.elementCount();
      const size_t byteCount = (elementCount * sizeof(jpfloat_t));
      unmapmem(_data, byteCount);
    } else if (_quantizedData) {
      const int elementCount = _dims.elementCount();
      const int sizeofElement = (_bitsPerElement / 8);
      const size_t byteCount = (elementCount * sizeofElement);
      unmapmem(_quantizedData, byteCount);
    }
    mem_unlock(_gpuMemoryHandle);
    mem_free(_gpuMemoryHandle);
#elif defined(USE_EIGEN_GEMM)
    Eigen::internal::aligned_free(_data);
    Eigen::internal::aligned_free(_quantizedData);
#else // TARGET_PI
    free(_data);
    free(_quantizedData);
#endif // TARGET_PI
  }
  if (_debugString)
  {
    free(_debugString);
  }
  if (_name)
  {
    free(_name);
  }
}

char* Buffer::debugString() {
  if (!_debugString) {
    _debugString = (char*)(malloc(MAX_DEBUG_STRING_LEN));
  }
  snprintf(_debugString, MAX_DEBUG_STRING_LEN, "Buffer %s - %s, %d bits per element, range (%f-%f)",
    _name, _dims.debugString(), _bitsPerElement, _min, _max);
  return _debugString;
}

void Buffer::printContents(int maxElements) {
  FILE* output = stderr;
  fprintf(output, "%s : \n", debugString());
  Dimensions dims = _dims;
  const int elementCount = dims.elementCount();
  const int bitsPerElement = _bitsPerElement;
  const jpfloat_t min = _min;
  const jpfloat_t max = _max;
  const jpfloat_t range = ((max - min) / (1 << bitsPerElement));
  if (elementCount < 5000) {
    maxElements = -1;
  }
  bool isSingleImage = (dims[0] == 1);
  if (isSingleImage) {
    dims = dims.removeDimensions(1);
  }
  const int dimsLength = dims._length;
  const jpfloat_t* const data = _data;
  if (isSingleImage) {
    fprintf(output, "[");
  }
  if (dimsLength == 1) {
    const int width = dims[0];
    int xLeft;
    int xRight;
    if (maxElements < 1) {
      xLeft = -1;
      xRight = -1;
    } else {
      xLeft = (maxElements / 2);
      xRight = (width - (maxElements / 2));
    }
    fprintf(output, "[");
    for (int x = 0; x < width; x += 1) {
      if (x == xLeft) {
        fprintf(output, "...");
      }
      if ((x >= xLeft) && (x < xRight)) {
        continue;
      }
      if (x > 0) {
        fprintf(output, ", ");
      }
      jpfloat_t value;
      if (bitsPerElement == 32) {
        value = *(data + dims.offset(x));
      } else if (bitsPerElement == 16) {
        uint16_t quantizedValue = *(((uint16_t*)(_quantizedData) + dims.offset(x)));
        value = (min + (quantizedValue * range));
      } else if (bitsPerElement == 8) {
        uint8_t quantizedValue = *(((uint8_t*)(_quantizedData) + dims.offset(x)));
        value = (min + (quantizedValue * range));
      } else {
        assert(false); // Should never get here
        value = 0.0f;
      }
      fprintf(output, "%.10f", value);
    }
    fprintf(output, "]");
  } else if (dimsLength == 2) {
    const int height = dims[0];
    const int width = dims[1];

    int yTop;
    int yBottom;
    if (maxElements < 1) {
      yTop = -1;
      yBottom = -1;
    } else {
      yTop = (maxElements / 2);
      yBottom = (height - (maxElements / 2));
    }

    int xLeft;
    int xRight;
    if (maxElements < 1) {
      xLeft = -1;
      xRight = -1;
    } else {
      xLeft = (maxElements / 2);
      xRight = (width - (maxElements / 2));
    }

    fprintf(output, "[");
    for (int y = 0; y < height; y += 1) {
      if (y == yTop) {
        fprintf(output, "...\n");
      }
      if ((y >= yTop) && (y < yBottom)) {
        continue;
      }
      fprintf(output, "[");
      for (int x = 0; x < width; x += 1) {
        if (x == xLeft) {
          fprintf(output, "...");
        }
        if ((x >= xLeft) && (x < xRight)) {
          continue;
        }
        if (x > 0) {
          fprintf(output, ", ");
        }
        jpfloat_t value;
        if (bitsPerElement == 32) {
          value = *(data + dims.offset(y, x));
        } else if (bitsPerElement == 16) {
          uint16_t quantizedValue = *(((uint16_t*)(_quantizedData) + dims.offset(y, x)));
          value = (min + (quantizedValue * range));
        } else if (bitsPerElement == 8) {
          uint8_t quantizedValue = *(((uint8_t*)(_quantizedData) + dims.offset(y, x)));
          value = (min + (quantizedValue * range));
        } else {
          assert(false); // Should never get here
          value = 0.0f;
        }
        fprintf(output, "%.10f", value);
      }
      if (y < (height - 1)) {
        fprintf(output, "],\n");
      } else {
        fprintf(output, "]");
      }
    }
    fprintf(output, "]");
  } else if (dimsLength == 3) {
    const int height = dims[0];
    const int width = dims[1];
    const int channels = dims[2];

    int yTop;
    int yBottom;
    if (maxElements < 1) {
      yTop = -1;
      yBottom = -1;
    } else {
      yTop = (maxElements / 2);
      yBottom = (height - (maxElements / 2));
    }

    int xLeft;
    int xRight;
    if (maxElements < 1) {
      xLeft = -1;
      xRight = -1;
    } else {
      xLeft = (maxElements / 2);
      xRight = (width - (maxElements / 2));
    }

    fprintf(output, "[");
    for (int y = 0; y < height; y += 1) {
      if (y == yTop) {
        fprintf(output, "...\n");
      }
      if ((y >= yTop) && (y < yBottom)) {
        continue;
      }
      fprintf(output, "[");
      for (int x = 0; x < width; x += 1) {
        if (x == xLeft) {
          fprintf(output, "...");
        }
        if ((x >= xLeft) && (x < xRight)) {
          continue;
        }
        if (x > 0) {
          fprintf(output, ", ");
        }
        fprintf(output, "(");
        for (int channel = 0; channel < channels; channel += 1) {
          jpfloat_t value;
          if (bitsPerElement == 32) {
            value = *(data + dims.offset(y, x, channel));
          } else if (bitsPerElement == 16) {
            uint16_t quantizedValue = *(((uint16_t*)(_quantizedData) + dims.offset(y, x, channel)));
            value = (min + (quantizedValue * range));
          } else if (bitsPerElement == 8) {
            uint8_t quantizedValue = *(((uint8_t*)(_quantizedData) + dims.offset(y, x, channel)));
            value = (min + (quantizedValue * range));
          } else {
            assert(false); // Should never get here
            value = 0.0f;
          }
          if (channel > 0) {
            fprintf(output, ", ");
          }
          fprintf(output, "%.10f", value);
        }
        fprintf(output, ")");
      }
      if (y < (height - 1)) {
        fprintf(output, "],\n");
      } else {
        fprintf(output, "]");
      }
    }
    fprintf(output, "]");
  } else {
    fprintf(output, "Printing of buffers with %d dimensions is not supported\n", dimsLength);
  }
  if (isSingleImage) {
    fprintf(output, "]");
  }
  fprintf(output, "\n");
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

bool Buffer::canReshapeTo(const Dimensions& newDims) {
  const int oldElementCount = _dims.elementCount();
  const int newElementCount = newDims.elementCount();
  const bool canReshape = (oldElementCount == newElementCount);
  return canReshape;
}

void Buffer::reshape(const Dimensions& newDims) {
  assert(canReshapeTo(newDims));
  _dims = newDims;
}

Buffer* Buffer::view() {
  Buffer* result = new Buffer(_dims, _data);
  char copyName[MAX_DEBUG_STRING_LEN];
  snprintf(copyName, MAX_DEBUG_STRING_LEN, "%s (view)", _name);
  result->setName(copyName);
#if defined(TARGET_PI)
  result->_gpuMemoryHandle = _gpuMemoryHandle;
  result->_gpuMemoryBase = _gpuMemoryBase;
#endif // TARGET_PI
  return result;
}

void Buffer::copyDataFrom(const Buffer* other) {
  const Dimensions& myDims = _dims;
  const Dimensions& otherDims = other->_dims;
  const size_t myElementCount = myDims.elementCount();
  const size_t otherElementCount = otherDims.elementCount();
  assert(myElementCount == otherElementCount);
  jpfloat_t* myData = _data;
  const jpfloat_t* otherData = other->_data;
  const size_t myByteCount = (myElementCount * sizeof(jpfloat_t));
  memcpy(myData, otherData, myByteCount);
}

void Buffer::convertFromChannelMajor(const Dimensions& expectedDims) {
  if (expectedDims._length < 4) {
    // No channels in the lower dimensional images, so skip any conversion
    return;
  }
  assert(canReshapeTo(expectedDims));
  const int imageCount = expectedDims[0];
  const int height = expectedDims[1];
  const int width = expectedDims[2];
  const int channels = expectedDims[3];

  Dimensions oldDims(imageCount, channels, height, width);

  const int elementCount = expectedDims.elementCount();
  const size_t byteCount = (elementCount * sizeof(jpfloat_t));
  jpfloat_t* newData = (jpfloat_t*)(malloc(byteCount));
  jpfloat_t* oldData = _data;

  for (int imageIndex = 0; imageIndex < imageCount; imageIndex += 1) {
    for (int y = 0; y < height; y += 1) {
      for (int x = 0; x < width; x += 1) {
        for (int channel = 0; channel < channels; channel += 1) {
          jpfloat_t* source = (oldData + oldDims.offset(imageIndex, channel, y, x));
          jpfloat_t* dest = (newData + expectedDims.offset(imageIndex, y, x, channel));
          *dest = *source;
        }
      }
    }
  }

  if (_doesOwnData) {
    free(_data);
  }

  _data = newData;
  _doesOwnData = true;
}

void Buffer::populateWithRandomValues(jpfloat_t min, jpfloat_t max) {
  const int elementCount = _dims.elementCount();
  const int bitsPerElement = _bitsPerElement;
  if (bitsPerElement == 32) {
    jpfloat_t* dataStart = _data;
    jpfloat_t* dataEnd = (_data + elementCount);
    jpfloat_t* data = dataStart;
    while (data < dataEnd) {
      // rand() is awful, but as long as we're using this for debugging
      // it should be good enough, and avoids dependencies.
      *data = (((max - min) * ((float)rand() / RAND_MAX)) + min);
      data += 1;
    }
  } else if (bitsPerElement == 16) {
    uint16_t* dataStart = (uint16_t*)(_quantizedData);
    uint16_t* dataEnd = (dataStart + elementCount);
    uint16_t* data = dataStart;
    const int integerMin = (((min - _min) * (1 << bitsPerElement)) / (_max - _min));
    const int integerMax = (((max - _min) * (1 << bitsPerElement)) / (_max - _min));
    while (data < dataEnd) {
      // rand() is awful, but as long as we're using this for debugging
      // it should be good enough, and avoids dependencies.
      *data = (((integerMax - integerMin) * ((float)rand() / RAND_MAX)) + integerMin);
      data += 1;
    }
  } else if (bitsPerElement == 8) {
    uint8_t* dataStart = (uint8_t*)(_quantizedData);
    uint8_t* dataEnd = (dataStart + elementCount);
    uint8_t* data = dataStart;
    const int integerMin = (((min - _min) * (1 << bitsPerElement)) / (_max - _min));
    const int integerMax = (((max - _min) * (1 << bitsPerElement)) / (_max - _min));
    while (data < dataEnd) {
      // rand() is awful, but as long as we're using this for debugging
      // it should be good enough, and avoids dependencies.
      *data = (((integerMax - integerMin) * ((float)rand() / RAND_MAX)) + integerMin);
      data += 1;
    }
  } else {
    assert(false); // Should never get here
  }
}

void Buffer::quantize(int bits) {
  jpfloat_t min = FLT_MAX;
  jpfloat_t max = -FLT_MAX;

  const int elementCount = _dims.elementCount();
  jpfloat_t* dataStart = _data;
  jpfloat_t* dataEnd = (_data + elementCount);
  jpfloat_t* data = dataStart;
  while (data < dataEnd) {
    const jpfloat_t value = *data;
    min = fminf(min, value);
    max = fmaxf(max, value);
    data += 1;
  }

  const int levels = (1 << bits);

  const jpfloat_t spread = ((max - min) / levels);
  const jpfloat_t recipSpread = (1.0f / fmaxf(0.00000001f, spread));

  fprintf(stderr, "min = %f, max = %f\n", min, max);

  data = dataStart;
  while (data < dataEnd) {
    const jpfloat_t value = *data;
    const int quantized = (int)roundf((value - min) * recipSpread);
    const jpfloat_t dequantized = ((quantized * spread) + min);
    *data = dequantized;
    data += 1;
  }

}

void Buffer::transpose() {
  const Dimensions& originalDims = _dims;
  assert(originalDims._length == 2); // expecting width x height
  const int originalHeight = originalDims[0];
  const int originalWidth = originalDims[1];
  const int bitsPerElement = _bitsPerElement;
  const int bytesPerElement = (bitsPerElement / 8);

  const int newHeight = originalWidth;
  const int newWidth = originalHeight;

  Dimensions newDims(newHeight, newWidth);

  const int elementCount = originalDims.elementCount();
  const size_t byteCount = (elementCount * bytesPerElement);
  void* newDataBytes = malloc(byteCount);

  if (bitsPerElement == 32) {
    jpfloat_t* originalData = _data;
    jpfloat_t* newData = (jpfloat_t*)(newDataBytes);
    for (int originalY = 0; originalY < originalHeight; originalY += 1) {
      for (int originalX = 0; originalX < originalWidth; originalX += 1) {
        jpfloat_t* source = (originalData + originalDims.offset(originalY, originalX));
        jpfloat_t* dest = (newData + newDims.offset(originalX, originalY));
        *dest = *source;
      }
    }
    _data = newData;
  } else if (bitsPerElement == 16) {
    uint16_t* originalData = (uint16_t*)(_quantizedData);
    uint16_t* newData = (uint16_t*)(newDataBytes);
    for (int originalY = 0; originalY < originalHeight; originalY += 1) {
      for (int originalX = 0; originalX < originalWidth; originalX += 1) {
        uint16_t* source = (originalData + originalDims.offset(originalY, originalX));
        uint16_t* dest = (newData + newDims.offset(originalX, originalY));
        *dest = *source;
      }
    }
    _quantizedData = newData;
  } else if (bitsPerElement == 8) {
    uint8_t* originalData = (uint8_t*)(_quantizedData);
    uint8_t* newData = (uint8_t*)(newDataBytes);
    for (int originalY = 0; originalY < originalHeight; originalY += 1) {
      for (int originalX = 0; originalX < originalWidth; originalX += 1) {
        uint8_t* source = (originalData + originalDims.offset(originalY, originalX));
        uint8_t* dest = (newData + newDims.offset(originalX, originalY));
        *dest = *source;
      }
    }
    _quantizedData = newData;
  } else {
    assert(false); // should never get here
  }

  if (_doesOwnData) {
    free(_data);
  }

  _dims = newDims;
  _doesOwnData = true;
}

Buffer* buffer_from_image_file(const char* filename)
{
  int inputWidth;
  int inputHeight;
  int inputChannels;

  const bool isRaw = string_ends_with(filename, ".raw");

  uint8_t* imageData;
  if (isRaw) {
    FILE* inputFile = fopen(filename, "rb");
    if (!inputFile) {
      fprintf(stderr, "jpcnn couldn't open '%s'\n", filename);
      return NULL;
    }
    fseek(inputFile, 0, 2);
    const size_t bytesInFile = ftell(inputFile);
    fseek(inputFile, 0, 0);
    const int imageSize = 256;
    const size_t bytesPerImage = (imageSize * imageSize * 3);
    const size_t bytesPerImagePlusLabel = (bytesPerImage + 4);
    const size_t imageCount = (bytesInFile / bytesPerImagePlusLabel);
    if ((imageCount * bytesPerImagePlusLabel) != bytesInFile) {
      fprintf(stderr, "Bad file size %zu for %s - expected %zux%dx%dx3 + %zux4\n",
        bytesInFile, filename, imageCount, imageSize, imageSize, imageCount);
      return NULL;
    }
    uint8_t* fileData = (uint8_t*)(malloc(bytesInFile));
    fread(fileData, bytesInFile, 1, inputFile);
    fclose(inputFile);

    uint8_t* channeledImageData = (fileData + (imageCount * 4));
    const size_t bytesPerChannel = (imageSize * imageSize);
    uint8_t* inputRed = (channeledImageData + (0 * bytesPerChannel));
    uint8_t* inputGreen = (channeledImageData + (1 * bytesPerChannel));
    uint8_t* inputBlue = (channeledImageData + (2 * bytesPerChannel));

    const int outputChannels = 4;
    const size_t bytesPerOutputImage = (bytesPerChannel * outputChannels);

    uint8_t* outputImageData = (uint8_t*)(malloc(bytesPerOutputImage));
    uint8_t* output = outputImageData;
    uint8_t* outputEnd = (outputImageData + bytesPerOutputImage);
    while (output < outputEnd) {
      *output = *inputRed;
      inputRed += 1;
      output += 1;
      *output = *inputGreen;
      inputGreen += 1;
      output += 1;
      *output = *inputBlue;
      inputBlue += 1;
      output += 1;
      if (outputChannels > 3) {
        *output = 255;
        output += 1;
      }
    }
    free(fileData);

    imageData = outputImageData;
    inputWidth = imageSize;
    inputHeight = imageSize;
    inputChannels = outputChannels;

  } else {
#ifdef USE_OS_IMAGE
    imageData = os_image_load_from_file(filename, &inputWidth, &inputHeight, &inputChannels, 0);
#else // USE_OS_IMAGE
    FILE* inputFile = fopen(filename, "rb");
    if (!inputFile) {
      fprintf(stderr, "jpcnn couldn't open '%s'\n", filename);
      return NULL;
    }
    imageData = stbi_load_from_file(inputFile, &inputWidth, &inputHeight, &inputChannels, 0);
    fclose(inputFile);
#endif // USE_OS_IMAGE
  }
  if (!imageData) {
    fprintf(stderr, "jpcnn couldn't read '%s'\n", filename);
    return NULL;
  }

  Dimensions dims(inputHeight, inputWidth, inputChannels);
  Buffer* buffer = new Buffer(dims);
  buffer->setName(filename);

  jpfloat_t* bufferCurrent = buffer->_data;
  jpfloat_t* bufferEnd = (buffer->_data + buffer->_dims.elementCount());
  uint8_t* imageCurrent = imageData;
  while (bufferCurrent != bufferEnd)
  {
    *bufferCurrent = *imageCurrent;
    bufferCurrent += 1;
    imageCurrent += 1;
  }

  if (isRaw) {
    free(imageData);
  } else {
#ifdef USE_OS_IMAGE
    os_image_free(imageData);
#else // USE_OS_IMAGE
    stbi_image_free(imageData);
#endif // USE_OS_IMAGE
  }

  return buffer;
}

Buffer* buffer_from_dump_file(const char* filename)
{
  SBinaryTag* mainDict = read_tag_from_file(filename, false);
  assert(mainDict != NULL);

  Buffer* result = buffer_from_tag_dict(mainDict, false);
  result->setName(filename);

  deallocate_file_tag(mainDict, false);

  return result;
}

Buffer* buffer_from_tag_dict(SBinaryTag* mainDict, bool skipCopy) {

  SBinaryTag* bitsPerFloatTag = get_tag_from_dict(mainDict, "float_bits");
  const uint32_t bitsPerFloat = bitsPerFloatTag->payload.jpuint;
  if ((bitsPerFloat != 32) && (bitsPerFloat != 16) && (bitsPerFloat != 8)) {
    fprintf(stderr, "jpcnn can only read 32, 16 or 8 bit dump files, found %d\n", bitsPerFloat);
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
  const int elementCount = dims.elementCount();

  Buffer* buffer;
  if (bitsPerFloat == 32) {
    SBinaryTag* dataTag = get_tag_from_dict(mainDict, "data");
    assert(dataTag->type == JP_FARY);
    assert(dataTag->length == (elementCount * sizeof(jpfloat_t)));
    if (skipCopy) {
      jpfloat_t* tagDataArray = dataTag->payload.jpfary;
      buffer = new Buffer(dims, tagDataArray);
    } else {
      buffer = new Buffer(dims);
      memcpy(buffer->_data, dataTag->payload.jpchar, dataTag->length);
    }
  } else {
    SBinaryTag* quantizedDataTag = get_tag_from_dict(mainDict, "quantized_data");
    const size_t sizeofElement = (bitsPerFloat / 8);
    assert(quantizedDataTag->type == JP_BLOB);
    assert(quantizedDataTag->length == (elementCount * sizeofElement));
    jpfloat_t min = get_float_from_dict(mainDict, "min");
    jpfloat_t max = get_float_from_dict(mainDict, "max");
    void* tagDataArray = quantizedDataTag->payload.jpchar;
    if (skipCopy) {
      buffer = new Buffer(dims, tagDataArray, min, max, bitsPerFloat);
    } else {
#ifdef LOAD_BUFFERS_AS_FLOAT
      buffer = new Buffer(dims);
      jpfloat_t range = ((max - min) / (1 << bitsPerFloat));
      const size_t elementsCount = dims.elementCount();
      if (bitsPerFloat == 16) {
        uint16_t* quantizedData = (uint16_t*)tagDataArray;
        jpfloat_t* floatData = buffer->_data;
#ifdef USE_ACCELERATE_GEMM
        vDSP_vfltu16(
          quantizedData,
          1,
          floatData,
          1,
          elementsCount);
        vDSP_vsmsa(
          floatData,
          1,
          &range,
          &min,
          floatData,
          1,
          elementsCount
        );
#else // USE_ACCELERATE_GEMM
        jpfloat_t* current = floatData;
        jpfloat_t* end = (current + elementsCount);
        uint16_t* quantized = quantizedData;
        while (current < end) {
          *current = (min + ((*quantized) * range));
          current += 1;
          quantized += 1;
        }
#endif // USE_ACCELERATE_GEMM
    } else if (bitsPerFloat == 8) {
        uint8_t* quantizedData = (uint8_t*)tagDataArray;
        jpfloat_t* floatData = buffer->_data;
#ifdef USE_ACCELERATE_GEMM
        vDSP_vfltu8(
          quantizedData,
          1,
          floatData,
          1,
          elementsCount);
        vDSP_vsmsa(
          floatData,
          1,
          &range,
          &min,
          floatData,
          1,
          elementsCount
        );
#else // USE_ACCELERATE_GEMM
        jpfloat_t* current = floatData;
        jpfloat_t* end = (current + elementsCount);
        uint8_t* quantized = quantizedData;
        while (current < end) {
          *current = (min + ((*quantized) * range));
          current += 1;
          quantized += 1;
        }
#endif // USE_ACCELERATE_GEMM
      } else {
        assert(false); // Should never get here, only 8 or 16 bit supported
      }
#else // LOAD_BUFFERS_AS_FLOAT
      buffer = new Buffer(dims, min, max, bitsPerFloat);
      memcpy(buffer->_quantizedData, tagDataArray, quantizedDataTag->length);
#endif // LOAD_BUFFERS_AS_FLOAT
    }
  }

  return buffer;
}

SBinaryTag* buffer_to_tag_dict(Buffer* buffer, int floatBits) {
  SBinaryTag* mainDict = create_dict_tag();
  mainDict = add_uint_to_dict(mainDict, "float_bits", floatBits);

  assert((floatBits == 32) || (floatBits == 16) || (floatBits == 8));

  SBinaryTag* dimsTag = create_list_tag();
  for (int index = 0; index < buffer->_dims._length; index += 1) {
    dimsTag = add_uint_to_list(dimsTag, buffer->_dims[index]);
  }
  mainDict = add_tag_to_dict(mainDict, "dims", dimsTag);
  free(dimsTag);

  if ((floatBits == 8) || (floatBits == 16)) {
    jpfloat_t min;
    jpfloat_t max;
    void* quantizedData;
    size_t sizeofQuantizedData;
    if (buffer->_bitsPerElement == 32) {
      quantize_buffer(buffer, floatBits, &min, &max, &quantizedData, &sizeofQuantizedData);
    } else {
      assert(floatBits == buffer->_bitsPerElement);
      quantizedData = buffer->_quantizedData;
      min = buffer->_min;
      max = buffer->_max;
      const int bytesPerElement = (buffer->_bitsPerElement / 8);
      const int elementCount = buffer->_dims.elementCount();
      sizeofQuantizedData = (bytesPerElement * elementCount);
    }

    mainDict = add_float_to_dict(mainDict, "min", min);
    mainDict = add_float_to_dict(mainDict, "max", max);
    mainDict = add_blob_to_dict(mainDict, "quantized_data", quantizedData, (int)sizeofQuantizedData);

    if (buffer->_bitsPerElement == 32) {
      free(quantizedData);
    }
  } else if (floatBits == 32) {
    mainDict = add_float_array_to_dict(mainDict, "data", buffer->_data, buffer->_dims.elementCount());
  }

  return mainDict;
}

void buffer_dump_to_file(Buffer* buffer, const char* filename) {
  SBinaryTag* mainDict = buffer_to_tag_dict(buffer);
  FILE* outputFile = fopen(filename, "wb");
  assert(outputFile != NULL);
  fwrite(mainDict, (mainDict->length + 8), 1, outputFile);
  fclose(outputFile);
  free(mainDict);
}

void buffer_save_to_image_file(Buffer* buffer, const char* basename) {
  const int maxFilenameLength = 1024;
  char filename[maxFilenameLength];

  const Dimensions dims = buffer->_dims;
  assert((dims._length == 4) || (dims._length == 3));

#ifdef USE_OS_IMAGE
  const char* imageSuffix = "png";
#else // USE_OS_IMAGE
  const char* imageSuffix = "ppm";
#endif // USE_OS_IMAGE

  if (dims._length == 3) {
    snprintf(filename, maxFilenameLength, "%s.%s", basename, imageSuffix);
    buffer_do_save_to_image_file(buffer, filename);
  } else {
    for (int index = 0; index < dims[0]; index += 1) {
      Buffer* view = buffer_view_at_top_index(buffer, index);
      snprintf(filename, maxFilenameLength, "%s_%02d.%s", basename, index, imageSuffix);
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

#ifdef USE_OS_IMAGE

  const size_t bytesPerRow = (width * channels);
  const size_t bytesPerImage = (height * bytesPerRow);
  uint8_t* pixelData = (uint8_t*)(malloc(bytesPerImage));
  for (int y = 0; y < height; y += 1) {
    uint8_t* rowData = (pixelData + (y * bytesPerRow));
    for (int x = 0; x < width; x += 1) {
      uint8_t* pixelData = (rowData + (x * channels));
      for (int channel = 0; channel < 3; channel += 1) {
        int value;
        if (channel >= channels) {
          value = 0;
        } else {
          value = (int)(*(data + dims.offset(y, x, channel)));
        }
        pixelData[channel] = value;
      }
    }
  }

  os_image_save_to_file(filename, pixelData, width, height, channels);

#else // USE_OS_IMAGE

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

#endif // USE_OS_IMAGE
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
  jpfloat_t* aEnd = (a->_data + a->_dims.elementCount());
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
#if defined(TARGET_PI)
  output->_gpuMemoryHandle = input->_gpuMemoryHandle;
  output->_gpuMemoryBase = input->_gpuMemoryBase;
#endif // TARGET_PI
  return output;
}

Buffer* convert_from_channeled_rgb_image(Buffer* input) {
  const Dimensions dims = input->_dims;
  assert(dims._length == 3);
  const int width = dims[1];
  const int height = dims[0];
  const int channels = dims[2];
  assert(channels == 3);

  Buffer* result = new Buffer(dims);

  jpfloat_t* inputData = input->_data;
  jpfloat_t* outputData = result->_data;

  const size_t bytesPerChannel = (width * height);
  const size_t bytesPerImage = (bytesPerChannel * channels);
  jpfloat_t* inputRed = (inputData + (0 * bytesPerChannel));
  jpfloat_t* inputGreen = (inputData + (1 * bytesPerChannel));
  jpfloat_t* inputBlue = (inputData + (2 * bytesPerChannel));

  jpfloat_t* output = outputData;
  jpfloat_t* outputEnd = (outputData + bytesPerImage);
  while (output < outputEnd) {
    *output = *inputRed;
    inputRed += 1;
    output += 1;
    *output = *inputGreen;
    inputGreen += 1;
    output += 1;
    *output = *inputBlue;
    inputBlue += 1;
    output += 1;
  }

  return result;
}

Buffer* convert_to_channeled_rgb_image(Buffer* input) {
  const Dimensions dims = input->_dims;
  assert(dims._length == 3);
  const int width = dims[1];
  const int height = dims[0];
  const int channels = dims[2];
  assert(channels == 3);

  Buffer* result = new Buffer(dims);

  jpfloat_t* inputData = input->_data;
  jpfloat_t* outputData = result->_data;

  const size_t bytesPerChannel = (width * height);
  const size_t bytesPerImage = (bytesPerChannel * channels);
  jpfloat_t* outputRed = (outputData + (0 * bytesPerChannel));
  jpfloat_t* outputGreen = (outputData + (1 * bytesPerChannel));
  jpfloat_t* outputBlue = (outputData + (2 * bytesPerChannel));

  jpfloat_t* currentInput = inputData;
  jpfloat_t* inputEnd = (inputData + bytesPerImage);
  while (currentInput < inputEnd) {
    *outputRed = *currentInput;
    outputRed += 1;
    currentInput += 1;
    *outputGreen = *currentInput;
    outputGreen += 1;
    currentInput += 1;
    *outputBlue = *currentInput;
    outputBlue += 1;
    currentInput += 1;
  }

  return result;
}

Buffer* extract_subregion(Buffer* input, const Offset& origin, const Dimensions& size) {
  const Dimensions& inputDims = input->_dims;
  assert(origin._length == size._length);
  assert(origin._length == 3); // We only handle this case right now
  assert(origin[2] == 0);
  assert(size[2] == inputDims[2]);

  const int inputWidth = inputDims[1];
  const int inputChannels = inputDims[2];

  const int regionWidth = size[1];
  const int regionChannels = size[2];

  const int bitsPerElement = (input->_bitsPerElement);
  const int bytesPerElement = (bitsPerElement / 8);

  Buffer* output;
  if (bitsPerElement == 32) {
    output = new Buffer(size);
  } else {
    output = new Buffer(size, input->_min, input->_max, bitsPerElement);
  }

  const size_t elementsPerInputRow = (inputWidth * inputChannels);
  const size_t bytesPerInputRow = (elementsPerInputRow * bytesPerElement);
  const size_t elementsPerRegionRow = (regionWidth * regionChannels);
  const size_t bytesPerRegionRow = (elementsPerRegionRow * bytesPerElement);

  const size_t inputOffset = inputDims.offset(origin[0], origin[1], origin[2]);
  const uint8_t* inputByteDataStart;
  if (bitsPerElement == 32) {
    inputByteDataStart = (uint8_t*)(input->_data);
  } else {
    inputByteDataStart = (uint8_t*)(input->_quantizedData);
  }
  const uint8_t* inputByteData = (inputByteDataStart + (inputOffset * bytesPerElement));
  uint8_t* regionByteData;
  if (bitsPerElement == 32) {
    regionByteData = (uint8_t*)(output->_data);
  } else {
    regionByteData = (uint8_t*)(output->_quantizedData);
  }
  uint8_t* const regionByteEnd = (regionByteData + (size.elementCount() * bytesPerElement));
  while (regionByteData < regionByteEnd) {
    memcpy(regionByteData, inputByteData, bytesPerRegionRow);
    inputByteData += bytesPerInputRow;
    regionByteData += bytesPerRegionRow;
  }

  return output;
}

void quantize_buffer(Buffer* input, int howManyBits, jpfloat_t* outMin, jpfloat_t* outMax, void** outData, size_t* outSizeofData) {
  assert((howManyBits == 8) || (howManyBits == 16));

  jpfloat_t min = FLT_MAX;
  jpfloat_t max = -FLT_MAX;

  const int elementCount = input->_dims.elementCount();
  jpfloat_t* dataStart = input->_data;
  jpfloat_t* dataEnd = (input->_data + elementCount);
  jpfloat_t* data = dataStart;
  while (data < dataEnd) {
    const jpfloat_t value = *data;
    min = fminf(min, value);
    max = fmaxf(max, value);
    data += 1;
  }

  const int levels = (1 << howManyBits);

  const jpfloat_t spread = ((max - min) / levels);
  const jpfloat_t recipSpread = (1.0f / fmaxf(0.00000001f, spread));

  if (howManyBits == 8) {
    const size_t sizeofQuantizedData = elementCount * sizeof(uint8_t);
    uint8_t* quantizedDataStart = (uint8_t*)(malloc(sizeofQuantizedData));
    data = dataStart;
    uint8_t* quantizedData = quantizedDataStart;
    while (data < dataEnd) {
      const jpfloat_t value = *data;
      int quantized = (int)roundf((value - min) * recipSpread);
      if (quantized < 0) {
        quantized = 0;
      } else if (quantized >= levels) {
        quantized = (levels - 1);
      }
      *quantizedData = (uint8_t)(quantized);
      data += 1;
      quantizedData += 1;
    }
    *outData = quantizedDataStart;
    *outSizeofData = sizeofQuantizedData;
  } else if (howManyBits == 16) {
    const size_t sizeofQuantizedData = elementCount * sizeof(uint16_t);
    uint16_t* quantizedDataStart = (uint16_t*)(malloc(sizeofQuantizedData));
    data = dataStart;
    uint16_t* quantizedData = quantizedDataStart;
    while (data < dataEnd) {
      const jpfloat_t value = *data;
      int quantized = (int)roundf((value - min) * recipSpread);
      if (quantized < 0) {
        quantized = 0;
      } else if (quantized >= levels) {
        quantized = (levels - 1);
      }
      *quantizedData = (uint16_t)(quantized);
      data += 1;
      quantizedData += 1;
    }
    *outData = quantizedDataStart;
    *outSizeofData = sizeofQuantizedData;
  } else {
    assert(false); // should never get here
    *outData = NULL;
    *outSizeofData = 0;
  }
  *outMin = min;
  *outMax = max;
}
