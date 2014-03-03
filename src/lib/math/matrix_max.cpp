//
//  matrix_max.cpp
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#include "matrix_ops.h"

#include <assert.h>
#include <math.h>
#include <float.h>

#include "buffer.h"

Buffer* matrix_max(Buffer* input, jpfloat_t maxValue) {
#ifdef DO_LOG_OPERATIONS
  fprintf(stderr, "matrix_max(input=[%s], maxValue=%f)\n",
    input->debugString(), maxValue);
#endif // DO_LOG_OPERATIONS

  const Dimensions inputDims = input->_dims;
  Buffer* output = new Buffer(inputDims);

  jpfloat_t* const outputDataStart = output->_data;
  jpfloat_t* const outputDataEnd = (outputDataStart + output->_dims.elementCount());
  const jpfloat_t* const inputDataStart = input->_data;
  jpfloat_t* outputData = outputDataStart;
  const jpfloat_t* inputData = inputDataStart;
  while (outputData != outputDataEnd) {
    const jpfloat_t inputValue = *inputData;
    *outputData = fmaxf(inputValue, maxValue);
    outputData += 1;
    inputData += 1;
  }

#ifdef DO_LOG_OPERATIONS
  fprintf(stderr, "matrix_max() result=[%s]\n",
    output->debugString());
#endif // DO_LOG_OPERATIONS

  return output;
}

Buffer* matrix_max_patch(Buffer* input, int patchWidth, int stride) {
#ifdef DO_LOG_OPERATIONS
  fprintf(stderr, "matrix_max_patch(input=[%s], patchWidth=%d, stride=%d)\n",
    input->debugString(), patchWidth, stride);
#endif // DO_LOG_OPERATIONS

  const Dimensions inputDims = input->_dims;
  // We're expecting (# of images, height, width, # of channels)
  assert(inputDims._length == 4);

  const int imageCount = inputDims[0];
  const int inputWidth = inputDims[2];
  const int inputHeight = inputDims[1];
  const int inputChannels = inputDims[3];

  const int outputWidth = (int)(floorf((inputWidth - patchWidth) / stride) + 1);
  const int outputHeight = (int)(floorf((inputHeight - patchWidth) / stride) + 1);
  const int outputChannels = inputChannels;
  const Dimensions outputDims(imageCount, outputHeight, outputWidth, outputChannels);
  Buffer* output = new Buffer(outputDims);

  for (int imageIndex = 0; imageIndex < imageCount; imageIndex += 1) {
    for (int outputY = 0; outputY < outputHeight; outputY += 1) {
      const int inputOriginY = (outputY * stride);
      for (int outputX = 0; outputX < outputWidth; outputX += 1) {
        const int inputOriginX = (outputX * stride);
        for (int outputChannel = 0; outputChannel < outputChannels; outputChannel += 1) {
          jpfloat_t patchMax = -FLT_MAX;
          for (int patchY = 0; patchY < patchWidth; patchY += 1) {
            const int inputY = (int)fmin((inputHeight - 1), (inputOriginY + patchY));
            for (int patchX = 0; patchX < patchWidth; patchX += 1) {
              const int inputX = (int)fmin((inputWidth - 1), (inputOriginX + patchX));
              const int inputOffset = inputDims.offset(imageIndex, inputY, inputX, outputChannel);
              const jpfloat_t inputValue = *(input->_data + inputOffset);
              patchMax = fmaxf(patchMax, inputValue);
            }
          }
          const int outputOffset = outputDims.offset(imageIndex, outputY, outputX, outputChannel);
          *(output->_data + outputOffset) = patchMax;
        }
      }
    }
  }

#ifdef DO_LOG_OPERATIONS
  fprintf(stderr, "matrix_max_patch() result=[%s]\n",
    output->debugString());
#endif // DO_LOG_OPERATIONS

  return output;
}