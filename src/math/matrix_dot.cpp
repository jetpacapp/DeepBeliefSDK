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

#ifdef USE_ACCELERATE_GEMM
#include <Accelerate/Accelerate.h>
#define USE_GEMM
#endif

#ifdef USE_MKL_GEMM
#include <mkl_cblas.h>
#define USE_GEMM
#endif // USE_MKL_GEMM

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

#ifdef USE_GEMM

  CBLAS_ORDER order = CblasColMajor;
  CBLAS_TRANSPOSE transposeA = CblasNoTrans;
  CBLAS_TRANSPOSE transposeB = CblasNoTrans;
  const int m = outputChannels;
  const int n = input->_dims[0];
  const int k = input->_dims[1];
  const float alpha = 1.0f;
  const int lda = m;
  const int ldb = k;
  const int ldc = m;
  const jpfloat_t beta = 0.0f;

  cblas_sgemm(
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

  return output;
}
