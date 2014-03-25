//
//  glgemm_test.cpp
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#ifdef USE_OPENGL

#include <assert.h>

#include "matrix_ops.h"
#include "buffer.h"
#include "glgemm.h"

int main(int argc, char** argv) {

  Buffer* input = new Buffer(Dimensions(1, 100));
  Buffer* weights = new Buffer(Dimensions(100, 20));

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
  Buffer* outputCPU = new Buffer(outputDims);
  outputCPU->setName("outputCPU");
  Buffer* outputGPU = new Buffer(outputDims);
  outputGPU->setName("outputGPU");

  input->populateWithRandomValues(0, 1);
  weights->populateWithRandomValues(0, 1);

  const int order = JPCblasColMajor;
  const int transposeA = JPCblasNoTrans;
  const int transposeB = JPCblasNoTrans;

  const int m = outputChannels;
  const int n = input->_dims[0];
  const int k = input->_dims[1];
  const float alpha = 1.0f;
  const int lda = m;
  const int ldb = k;
  const int ldc = m;
  const jpfloat_t beta = 0.0f;

  naive_cblas_sgemm(
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
    outputCPU->_data,
    ldc
  );

  gl_gemm(
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
    outputGPU->_data,
    ldc
  );

  outputCPU->printContents();
  outputGPU->printContents();

  const float weightsMin = 0.0f;
  const float weightsMax = 1.0f;
  const int weightsBitsPerElement = 16;

  Buffer* weightsFixed = new Buffer(Dimensions(100, 20), weightsMin, weightsMax, weightsBitsPerElement);
  Buffer* outputFixedCPU = new Buffer(outputDims);
  outputFixedCPU->setName("outputFixedCPU");
  Buffer* outputFixedGPU = new Buffer(outputDims);
  outputFixedGPU->setName("outputFixedGPU");

  weightsFixed->populateWithRandomValues(0, 1);

  naive_cblas_sgemm_fixed(
    order,
    transposeA,
    transposeB,
    m,
    n,
    k,
    alpha,
    weightsFixed->_quantizedData,
    weightsMin,
    weightsMax,
    weightsBitsPerElement,
    lda,
    input->_data,
    ldb,
    beta,
    outputFixedCPU->_data,
    ldc
  );

  gl_gemm_fixed(
    order,
    transposeA,
    transposeB,
    m,
    n,
    k,
    alpha,
    weightsFixed->_quantizedData,
    weightsMin,
    weightsMax,
    weightsBitsPerElement,
    lda,
    input->_data,
    ldb,
    beta,
    outputFixedGPU->_data,
    ldc
  );

  outputFixedCPU->printContents();
  outputFixedGPU->printContents();

  return 0;
}

#endif // USE_OPENGL