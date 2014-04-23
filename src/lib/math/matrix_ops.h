//
//  matrix_ops.h
//  jpcnn
//
//  The caller is responsible for delete-ing any returned Buffer*'s
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#ifndef INCLUDE_MATRIX_OPS_H
#define INCLUDE_MATRIX_OPS_H

#include "jpcnn.h"

class Buffer;

void matrix_add_inplace(Buffer* output, Buffer* input, jpfloat_t inputScale);
Buffer* matrix_correlate(Buffer* input, Buffer* kernels, int kernelWidth, int kernelCount, int stride, bool areKernelsTransposed);
Buffer* matrix_dot(Buffer* a, Buffer* b, bool areWeightsTransposed);
Buffer* matrix_extract_channels(Buffer* input, int startChannel, int endChannel);
Buffer* matrix_insert_margin(Buffer* input, int marginWidth, int marginHeight);
Buffer* matrix_join_channels(Buffer** inputs, int inputsCount);
Buffer* matrix_local_response(Buffer* input, int windowSize, jpfloat_t k, jpfloat_t alpha, jpfloat_t beta);
Buffer* matrix_max(Buffer* input, jpfloat_t maxValue);
Buffer* matrix_max_patch(Buffer* input, int patchWidth, int stride);
void matrix_scale_inplace(Buffer* output, jpfloat_t scale);
Buffer* matrix_softmax(Buffer* input);

enum JPCBLAS_ORDER {
  JPCblasRowMajor=101,
  JPCblasColMajor=102
};
enum JPCBLAS_TRANSPOSE {
  JPCblasNoTrans=111,
  JPCblasTrans=112,
  JPCblasConjTrans=113,
  JPAtlasConj=114
};

void matrix_gemm(
  int order,
  int transposeA,
  int transposeB,
  int m,
  int n,
  int k,
  jpfloat_t alpha,
  jpfloat_t *a,
  int lda,
  jpfloat_t *b,
  int ldb,
  jpfloat_t beta,
  jpfloat_t* c,
  int ldc);

void matrix_gemm_fixed(
  int order,
  int transposeA,
  int transposeB,
  int m,
  int n,
  int k,
  jpfloat_t alpha,
  void *a,
  jpfloat_t aMin,
  jpfloat_t aMax,
  int aBitsPerElement,
  int lda,
  jpfloat_t *b,
  int ldb,
  jpfloat_t beta,
  jpfloat_t* c,
  int ldc);

void naive_cblas_sgemm(
  int order,
  int transposeA,
  int transposeB,
  int m,
  int n,
  int k,
  jpfloat_t alpha,
  jpfloat_t *a,
  int lda,
  jpfloat_t *b,
  int ldb,
  jpfloat_t beta,
  jpfloat_t* c,
  int ldc);

void naive_cblas_sgemm_fixed(
  int order,
  int transposeA,
  int transposeB,
  int m,
  int n,
  int k,
  jpfloat_t alpha,
  void *a,
  jpfloat_t aMin,
  jpfloat_t aMax,
  int aBitsPerElement,
  int lda,
  jpfloat_t *b,
  int ldb,
  jpfloat_t beta,
  jpfloat_t* c,
  int ldc);

void cblas_sgemm_fixed(
  int order,
  int transposeA,
  int transposeB,
  int m,
  int n,
  int k,
  jpfloat_t alpha,
  void *a,
  jpfloat_t aMin,
  jpfloat_t aMax,
  int aBitsPerElement,
  int lda,
  jpfloat_t *b,
  int ldb,
  jpfloat_t beta,
  jpfloat_t* c,
  int ldc);

void eigen_cblas_sgemm(
  int order,
  int transposeA,
  int transposeB,
  int m,
  int n,
  int k,
  jpfloat_t alpha,
  jpfloat_t *a,
  int lda,
  jpfloat_t *b,
  int ldb,
  jpfloat_t beta,
  jpfloat_t* c,
  int ldc);

#endif // INCLUDE_MATRIX_OPS_H
