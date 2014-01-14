//
//  matrix_dot.cpp
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#include "matrix_ops.h"

#include <assert.h>

#include "buffer.h"

Buffer* matrix_dot(Buffer* input, Buffer* weights) {

  const Dimensions inputDims = input->_dims;
  // We're expecting (# of images, # of values)
  assert(inputDims._length == 2);

  const int imageCount = inputDims[0];
  const int inputValuesCount = inputDims[1];

  const Dimensions weightsDims = weights->_dims;
  // We're expecting (# of values in input, # of output channels)
  assert(inputDims._length == 2);
  assert(weightsDims[0] == inputValuesCount);
  const int outputChannels = weightsDims[1];

  const Dimensions outputDims(imageCount, outputChannels);
  Buffer* output = new Buffer(outputDims);

  const jpfloat_t* const weightsDataStart = weights->_data;
  jpfloat_t* const outputDataStart = output->_data;

  const int valuesPerWeightsRow = outputChannels;

  jpfloat_t* outputData = outputDataStart;
  for (int imageIndex = 0; imageIndex < imageCount; imageIndex += 1) {
    const jpfloat_t* const inputDataStart = (input->_data + (imageIndex * inputValuesCount));
    const jpfloat_t* const inputDataEnd = (inputDataStart + inputValuesCount);
    for (int outputChannel = 0; outputChannel < outputChannels; outputChannel += 1) {
      jpfloat_t accumulated = 0.0f;
      const jpfloat_t* inputData = inputDataStart;
      const jpfloat_t* weightsData = (weightsDataStart + outputChannel);
      while (inputData < inputDataEnd) {
        const jpfloat_t inputValue = *inputData;
        const jpfloat_t weightValue = *weightsData;
        accumulated += (inputValue * weightValue);
        inputData += 1;
        weightsData += valuesPerWeightsRow;
      }
      *outputData = accumulated;
      outputData += 1;
    }
  }

  return output;
}
