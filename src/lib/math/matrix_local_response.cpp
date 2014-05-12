//
//  matrix_local_response.cpp
//  jpcnn
//
//  Implements the local response normalization formula described in:
//  http://www.cs.toronto.edu/~hinton/absps/imagenet.pdf
//  Calculates a rolling average magnitude across the channels, and then
//  scales down channels based on their neighbors' strengths.
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#include "matrix_ops.h"

#include <assert.h>
#include <math.h>
#include <float.h>
#include <string.h>

#ifdef USE_ACCELERATE_GEMM
#include <Accelerate/Accelerate.h>
#endif // USE_ACCELERATE_GEMM

#ifdef USE_MKL_GEMM
#include <mkl_vml_functions.h>
#endif // USE_MKL_GEMM

#include "buffer.h"

Buffer* matrix_local_response(Buffer* input, int windowSize, jpfloat_t k, jpfloat_t alpha, jpfloat_t beta) {
#ifdef DO_LOG_OPERATIONS
  fprintf(stderr, "matrix_local_response(input=[%s], windowSize=%d, k=%f, alpha=%f, beta=%f)\n",
    input->debugString(), windowSize, k, alpha, beta);
#endif // DO_LOG_OPERATIONS

  const Dimensions inputDims = input->_dims;
  // We're expecting (# of images, height, width, # of channels)
  assert(inputDims._length == 4);

  const int inputChannels = inputDims[3];

  Buffer* magnitude = new Buffer(inputDims);

  Buffer* magBuffer = new Buffer(Dimensions(inputChannels));
  jpfloat_t* magBufferData = magBuffer->_data;

  const jpfloat_t* inputData = input->_data;
  const int elementCount = inputDims.elementCount();
  const jpfloat_t* inputDataEnd = (input->_data + elementCount);
  jpfloat_t* magnitudeData = magnitude->_data;

  const jpfloat_t alphaOverSize = (alpha / windowSize);
  const int prereadCount = ((windowSize / 2) - 0);

  while (inputData < inputDataEnd) {

    for (int channel = 0; channel < inputChannels; channel += 1) {
      const jpfloat_t inputValue = inputData[channel];
      magBufferData[channel] = (inputValue * inputValue * alphaOverSize);
    }

    float averagedScale = 0;
    for (int index = 0; index < prereadCount; index += 1) {
      averagedScale += magBufferData[index];
    }

    for (int channel = 0; channel < inputChannels; channel += 1) {
      const int rightIndex = (channel + (windowSize / 2));
      if (rightIndex < inputChannels) {
        averagedScale += magBufferData[rightIndex];
      }
      magnitudeData[channel] = (averagedScale + k);
      const int leftIndex = (channel - (windowSize / 2));
      if (leftIndex >= 0) {
        averagedScale -= magBufferData[leftIndex];
      }
    }

    inputData += inputChannels;
    magnitudeData += inputChannels;
  }

  delete magBuffer;

  Buffer* output = new Buffer(inputDims);

  inputData = input->_data;
  magnitudeData = magnitude->_data;
  jpfloat_t* outputData = output->_data;
#if defined(USE_ACCELERATE_GEMM)
  jpfloat_t* repeatedBeta = (jpfloat_t*)(malloc(sizeof(jpfloat_t) * elementCount));
  const float minusBeta = -beta;
  vDSP_vfill(&minusBeta, repeatedBeta, 1, elementCount);
  vvpowf(outputData, repeatedBeta, magnitudeData, &elementCount);
  free(repeatedBeta);
  vDSP_vmul(outputData, 1, inputData, 1, outputData, 1, elementCount);
#elif defined(USE_MKL_GEMM)
  jpfloat_t* repeatedBeta = (jpfloat_t*)(malloc(sizeof(jpfloat_t) * elementCount));
  const float minusBeta = -beta;
  jpfloat_t* betaCurrent = repeatedBeta;
  const jpfloat_t* const betaEnd = (repeatedBeta + elementCount);
  while (betaCurrent < betaEnd) {
    *betaCurrent = minusBeta;
    betaCurrent += 1;
  }
  vsPow(elementCount, magnitudeData, repeatedBeta, outputData);
  free(repeatedBeta);
  vsMul(elementCount, inputData, outputData, outputData);
#else // USE_ACCELERATE_GEMM
  while (inputData < inputDataEnd) {

    const jpfloat_t inputValue = *inputData;
    const jpfloat_t magnitudeValue = *magnitudeData;

    jpfloat_t outputValue = (powf(magnitudeValue, -beta) * inputValue);
    *outputData = outputValue;

    inputData += 1;
    magnitudeData += 1;
    outputData += 1;
  }
#endif // USE_ACCELERATE_GEMM

  delete magnitude;

#ifdef DO_LOG_OPERATIONS
  fprintf(stderr, "matrix_local_response() result=[%s]\n",
    output->debugString());
#endif // DO_LOG_OPERATIONS

  return output;
}