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

#ifdef USE_ACCELERATE_GEMM
#include <Accelerate/Accelerate.h>
#define USE_GEMM
#endif

#ifdef USE_MKL_GEMM
#include <mkl_cblas.h>
#define USE_GEMM
#endif // USE_MKL_GEMM

#ifdef USE_GEMM

static Buffer* patches_into_rows(Buffer* input, int kernelWidth, int stride);

Buffer* patches_into_rows(Buffer* input, int kernelWidth, int stride) {
  const Dimensions inputDims = input->_dims;
  // We're expecting (# of images, height, width, # of channels)
  assert(inputDims._length == 4);

  const int imageCount = inputDims[0];
  const int inputWidth = inputDims[2];
  const int inputHeight = inputDims[1];
  const int inputChannels = inputDims[3];

  const int pixelsPerKernel = (kernelWidth * kernelWidth);
  const int valuesPerKernel = (pixelsPerKernel * inputChannels);

  const int patchesAcross = (int)(floorf((inputWidth - kernelWidth) / stride) + 1);
  const int patchesDown = (int)(floorf((inputHeight - kernelWidth) / stride) + 1);
  const Dimensions outputDims(imageCount, (patchesDown * patchesAcross), valuesPerKernel);
  Buffer* output = new Buffer(outputDims);

  const jpfloat_t* const inputStart = input->_data;
  jpfloat_t* outputData = output->_data;

  const int valuesPerInputRow = inputDims.removeDimensions(2).elementCount();
  const int valuesPerKernelRow = (kernelWidth * inputChannels);
  const size_t bytesPerKernelRow = (valuesPerKernelRow * sizeof(jpfloat_t));

  for (int imageIndex = 0; imageIndex < imageCount; imageIndex += 1) {
    for (int patchY = 0; patchY < patchesDown; patchY += 1) {
      const int inputOriginY = (patchY * stride);
      for (int patchX = 0; patchX < patchesAcross; patchX += 1) {
        const int inputOriginX = (patchX * stride);
        const int inputPatchOffset = inputDims.offset(imageIndex, inputOriginY, inputOriginX, 0);
        const jpfloat_t* const inputPatchStart = (inputStart + inputPatchOffset);
        const jpfloat_t* inputData = inputPatchStart;
        for (int row = 0; row < kernelWidth; row += 1) {
          memcpy(outputData, inputData, bytesPerKernelRow);
          outputData += valuesPerKernelRow;
          inputData += valuesPerInputRow;
        }
      }
    }
  }

  return output;
}

Buffer* matrix_correlate(Buffer* input, Buffer* kernels, int kernelWidth, int kernelCount, int stride) {
  const Dimensions inputDims = input->_dims;
  // We're expecting (# of images, height, width, # of channels)
  assert(inputDims._length == 4);

  const int imageCount = inputDims[0];
  const int inputWidth = inputDims[2];
  const int inputHeight = inputDims[1];
  const int inputChannels = inputDims[3];

  const int pixelsPerKernel = (kernelWidth * kernelWidth);
  const int valuesPerKernel = (pixelsPerKernel * inputChannels);
  Dimensions expectedKernelsDims(valuesPerKernel, kernelCount);
  assert(expectedKernelsDims == kernels->_dims);

  const int outputWidth = (int)(floorf((inputWidth - kernelWidth) / stride) + 1);
  const int outputHeight = (int)(floorf((inputHeight - kernelWidth) / stride) + 1);
  const int outputChannels = kernelCount;
  const Dimensions outputDims(imageCount, outputHeight, outputWidth, outputChannels);
  Buffer* output = new Buffer(outputDims);

  Buffer* patches = patches_into_rows(input, kernelWidth, stride);

  CBLAS_ORDER order = CblasColMajor;
  CBLAS_TRANSPOSE transposeA = CblasNoTrans;
  CBLAS_TRANSPOSE transposeB = CblasNoTrans;
  const int m = kernelCount;
  const int n = (patches->_dims[1] * patches->_dims[0]);
  const int k = patches->_dims[2];
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
    kernels->_data,
    lda,
    patches->_data,
    ldb,
    beta,
    output->_data,
    ldc
  );

  delete patches;

  return output;
}

#else // Use the naive algorithm

Buffer* matrix_correlate(Buffer* input, Buffer* kernels, int kernelWidth, int kernelCount, int stride) {

  const Dimensions inputDims = input->_dims;
  // We're expecting (# of images, height, width, # of channels)
  assert(inputDims._length == 4);

  const int imageCount = inputDims[0];
  const int inputWidth = inputDims[2];
  const int inputHeight = inputDims[1];
  const int inputChannels = inputDims[3];

  const int pixelsPerKernel = (kernelWidth * kernelWidth);
  const int valuesPerKernel = (pixelsPerKernel * inputChannels);
  Dimensions expectedKernelsDims(valuesPerKernel, kernelCount);
  assert(expectedKernelsDims == kernels->_dims);

  const int outputWidth = (int)(floorf((inputWidth - kernelWidth) / stride) + 1);
  const int outputHeight = (int)(floorf((inputHeight - kernelWidth) / stride) + 1);
  const int outputChannels = kernelCount;
  const Dimensions outputDims(imageCount, outputHeight, outputWidth, outputChannels);
  Buffer* output = new Buffer(outputDims);

  for (int imageIndex = 0; imageIndex < imageCount; imageIndex += 1) {
    for (int outputY = 0; outputY < outputHeight; outputY += 1) {
      const int inputOriginY = (outputY * stride);
      for (int outputX = 0; outputX < outputWidth; outputX += 1) {
        const int inputOriginX = (outputX * stride);
        for (int outputChannel = 0; outputChannel < outputChannels; outputChannel += 1) {
          jpfloat_t accumulated = 0.0f;
          for (int kernelY = 0; kernelY < kernelWidth; kernelY += 1) {
            const int inputY = (inputOriginY + kernelY);
            for (int kernelX = 0; kernelX < kernelWidth; kernelX += 1) {
              const int inputX = (inputOriginX + kernelX);
              for (int kernelChannel = 0; kernelChannel < inputChannels; kernelChannel += 1) {
                const int kernelsOffset = (
                  (kernelY * kernelWidth * inputChannels * kernelCount) +
                  (kernelX * inputChannels * kernelCount) +
                  (kernelChannel * kernelCount) +
                  outputChannel);
                const jpfloat_t kernelValue = *(kernels->_data + kernelsOffset);
                const int inputOffset = inputDims.offset(imageIndex, inputY, inputX, kernelChannel);
                const jpfloat_t inputValue = *(input->_data + inputOffset);
                accumulated += (kernelValue * inputValue);
              }
            }
          }
          const int outputOffset = outputDims.offset(imageIndex, outputY, outputX, outputChannel);
          *(output->_data + outputOffset) = accumulated;
        }
      }
    }
  }

  return output;
}

#endif // USE_GEMM