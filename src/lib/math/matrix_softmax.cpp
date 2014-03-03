//
//  matrix_add.cpp
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#include "matrix_ops.h"

#include <assert.h>
#include <float.h>
#include <math.h>

#include "buffer.h"

Buffer* matrix_softmax(Buffer* input) {

#ifdef DO_LOG_OPERATIONS
  fprintf(stderr, "matrix_softmax(input=[%s])\n", input->debugString());
#endif // DO_LOG_OPERATIONS

  const Dimensions inputDims = input->_dims;
  // We're expecting (# of images, # of values)
  assert(inputDims._length == 2);

  const int imageCount = inputDims[0];
  const int inputValuesCount = inputDims[1];

  Buffer* output = new Buffer(inputDims);

  for (int imageIndex = 0; imageIndex < imageCount; imageIndex += 1) {
    const int imageOffset = (imageIndex * inputValuesCount);
    const jpfloat_t* const inputDataStart = (input->_data + imageOffset);
    const jpfloat_t* const inputDataEnd = (inputDataStart + inputValuesCount);
    jpfloat_t* const outputDataStart = (output->_data + imageOffset);
    jpfloat_t* const outputDataEnd = (outputDataStart + inputValuesCount);

    // Rescales the array to accentuate the positive, see here for details:
    // http://stackoverflow.com/questions/9906136/implementation-of-a-softmax-activation-function-for-neural-networks
    jpfloat_t max = -FLT_MAX;
    const jpfloat_t* inputData = inputDataStart;
    while (inputData < inputDataEnd) {
      const jpfloat_t inputValue = *inputData;
      max = fmaxf(max, inputValue);
      inputData += 1;
    }

    jpfloat_t sum = 0;
    inputData = inputDataStart;
    jpfloat_t* outputData = outputDataStart;
    while (inputData < inputDataEnd) {
      const jpfloat_t inputValue = *inputData;
      const jpfloat_t normalized = (inputValue - max);
      const jpfloat_t outputValue = exp(normalized);
      *outputData = outputValue;
      sum += outputValue;
      inputData += 1;
      outputData += 1;
    }

    jpfloat_t recipSum = (1.0 / sum);

    outputData = outputDataStart;
    while (outputData < outputDataEnd) {
      *outputData *= recipSum;
      outputData += 1;
    }
  }

#ifdef DO_LOG_OPERATIONS
  fprintf(stderr, "matrix_soft_max() result=[%s]\n",
    output->debugString());
#endif // DO_LOG_OPERATIONS

  return output;
}
