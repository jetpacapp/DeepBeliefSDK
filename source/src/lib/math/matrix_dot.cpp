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

#if defined(USE_QPU_GEMM)
#include "qpu_gemm.h"
#endif // USE_QPU_GEMM

Buffer* matrix_dot(Buffer* input, Buffer* weights, bool areWeightsTransposed) {

#ifdef DO_LOG_OPERATIONS
  fprintf(stderr, "matrix_dot(input=[%s], weights=[%s])\n",
    input->debugString(), weights->debugString());
#endif // DO_LOG_OPERATIONS

  const Dimensions inputDims = input->_dims;
  // We're expecting (# of images, # of values)
  assert(inputDims._length == 2);

  const int imageCount = inputDims[0];
  const int inputValuesCount = inputDims[1];

  const Dimensions weightsDims = weights->_dims;
  // We're expecting (# of values in input, # of output channels)
  assert(inputDims._length == 2);
  int inputValuesIndex;
  int outputChannelsIndex;
  if (areWeightsTransposed) {
    inputValuesIndex = 1;
    outputChannelsIndex = 0;
  } else {
    inputValuesIndex = 0;
    outputChannelsIndex = 1;
  }
  assert(weightsDims[inputValuesIndex] == inputValuesCount);
  const int outputChannels = weightsDims[outputChannelsIndex];

  const Dimensions outputDims(imageCount, outputChannels);
  Buffer* output = new Buffer(outputDims);

#ifdef USE_GEMM

  const int order = JPCblasColMajor;
  int transposeA;
  if (areWeightsTransposed) {
    transposeA = JPCblasTrans;
  } else {
    transposeA = JPCblasNoTrans;
  }
  const int transposeB = JPCblasNoTrans;

  const int m = outputChannels;
  const int n = input->_dims[0];
  const int k = input->_dims[1];
  const float alpha = 1.0f;
  int lda;
  if (areWeightsTransposed) {
    lda = k;
  } else {
    lda = m;
  }
  const int ldb = k;
  const int ldc = m;
  const jpfloat_t beta = 0.0f;

  if (weights->_bitsPerElement == 32) {
#if !defined(USE_QPU_GEMM)
    matrix_gemm(
      order,
      transposeA,
      transposeB,
      m,
      n,
      k,
      alpha,
      weights->_data,
      lda,
      input->_data,
      ldb,
      beta,
      output->_data,
      ldc
    );
#else // USE_QPU_GEMM
    qpu_cblas_sgemm(
      order,
      transposeA,
      transposeB,
      m,
      n,
      k,
      alpha,
      weights->_gpuMemoryBase,
      lda,
      input->_gpuMemoryBase,
      ldb,
      beta,
      output->_gpuMemoryBase,
      ldc
    );
#endif // USE_QPU_GEMM
  } else {
#if !defined(USE_QPU_GEMM)
    matrix_gemm_fixed(
      order,
      transposeA,
      transposeB,
      m,
      n,
      k,
      alpha,
      weights->_quantizedData,
      weights->_min,
      weights->_max,
      weights->_bitsPerElement,
      lda,
      input->_data,
      ldb,
      beta,
      output->_data,
      ldc
    );
#else // USE_QPU_GEMM
    qpu_cblas_sgemm_fixed(
      order,
      transposeA,
      transposeB,
      m,
      n,
      k,
      alpha,
      weights->_gpuMemoryBase,
      weights->_min,
      weights->_max,
      weights->_bitsPerElement,
      lda,
      input->_gpuMemoryBase,
      ldb,
      beta,
      output->_gpuMemoryBase,
      ldc
    );
#endif // USE_QPU_GEMM
  }

#else // Use naive algorithm instead

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
#endif // USE_GEMM

#ifdef DO_LOG_OPERATIONS
  fprintf(stderr, "matrix_dot() result=[%s]\n",
    output->debugString());
#endif // DO_LOG_OPERATIONS

  return output;
}
