//
//  matrix_correlate.cpp
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#include "matrix_ops.h"

#include <assert.h>
#include <math.h>
#include <string.h>

#include "buffer.h"
#include "dimensions.h"

Buffer* matrix_insert_margin(Buffer* input, int marginWidth, int marginHeight) {

#ifdef DO_LOG_OPERATIONS
  fprintf(stderr, "matrix_insert_margin(input=[%s], marginWidth=%d, marginHeight=%d)\n",
    input->debugString(), marginWidth, marginHeight);
#endif // DO_LOG_OPERATIONS

  const Dimensions inputDims = input->_dims;
  // We're expecting (# of images, height, width, # of channels)
  assert(inputDims._length == 4);

  const int imageCount = inputDims[0];
  const int inputWidth = inputDims[2];
  const int inputHeight = inputDims[1];
  const int inputChannels = inputDims[3];

  const int outputWidth = (inputWidth + (marginWidth * 2));
  const int outputHeight = (inputHeight + (marginHeight * 2));
  const Dimensions outputDims(imageCount, outputHeight, outputWidth, inputChannels);
  Buffer* output = new Buffer(outputDims);

  const int valuesPerInputRow = (inputWidth * inputChannels);
  const size_t bytesPerInputRow = (valuesPerInputRow * sizeof(jpfloat_t));
  const int valuesPerOutputRow = (outputWidth * inputChannels);
  const size_t bytesPerOutputRow = (valuesPerOutputRow * sizeof(jpfloat_t));

  const int valuesPerOutputMargin = (marginWidth * inputChannels);
  const int bytesPerOutputMargin = (valuesPerOutputMargin * sizeof(jpfloat_t));

  jpfloat_t* outputData = output->_data;
  jpfloat_t* inputData = input->_data;

  for (int imageIndex = 0; imageIndex < imageCount; imageIndex += 1) {
    for (int outputY = 0; outputY < outputHeight; outputY += 1) {
      const int inputOriginY = (outputY - marginHeight);
      if ((inputOriginY < 0) || (inputOriginY >= inputHeight)) {
        memset(outputData, 0, bytesPerOutputRow);
        outputData += valuesPerOutputRow;
      } else {
        memset(outputData, 0, bytesPerOutputMargin);
        outputData += valuesPerOutputMargin;
        memcpy(outputData, inputData, bytesPerInputRow);
        outputData += valuesPerInputRow;
        inputData += valuesPerInputRow;
        memset(outputData, 0, bytesPerOutputMargin);
        outputData += valuesPerOutputMargin;
      }
    }
  }

#ifdef DO_LOG_OPERATIONS
  fprintf(stderr, "matrix_insert_margin() result=[%s]\n",
    output->debugString());
#endif // DO_LOG_OPERATIONS

  return output;
}