//
//  matrix_gemm.cpp
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#include "matrix_ops.h"

#include <stdio.h>
#include <assert.h>

#ifdef USE_ACCELERATE_GEMM
#include <Accelerate/Accelerate.h>
#endif

#ifdef USE_MKL_GEMM
#include <mkl_cblas.h>
#endif // USE_MKL_GEMM

#ifdef USE_ATLAS_GEMM
#include <cblas.h>
#endif // USE_ATLAS_GEMM

#ifdef USE_OPENGL
#include "glgemm.h"
#endif // USE_OPENGL

#if !defined(USE_ACCELERATE_GEMM) && !defined(USE_MKL_GEMM) && !defined(USE_OPENGL) && !defined(USE_ATLAS_GEMM)
#define USE_NAIVE_GEMM
#endif

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
  int ldc) {

#ifdef DO_LOG_OPERATIONS
  fprintf(stderr, "matrix_gemm(\n  m=%d,\n  n=%d,\n  k=%d,\n  alpha=%f,\n  a=%p,\n  lda=%d,\n  b=%p,\n  ldb=%d,\n  beta=%f,\n  c=%p,\n  ldc=%d)\n",
    m,
    n,
    k,
    alpha,
    a,
    lda,
    b,
    ldb,
    beta,
    c,
    ldc);
#endif // DO_LOG_OPERATIONS

#if defined(USE_NAIVE_GEMM)
  naive_cblas_sgemm(
    order,
    transposeA,
    transposeB,
    m,
    n,
    k,
    alpha,
    a,
    lda,
    b,
    ldb,
    beta,
    c,
    ldc
  );
#elif defined(USE_OPENGL)
  gl_gemm(
    order,
    transposeA,
    transposeB,
    m,
    n,
    k,
    alpha,
    a,
    lda,
    b,
    ldb,
    beta,
    c,
    ldc
  );
#elif defined(USE_ACCELERATE_GEMM) || defined(USE_MKL_GEMM) || defined(USE_ATLAS_GEMM)
  cblas_sgemm(
    (enum CBLAS_ORDER)(order),
    (enum CBLAS_TRANSPOSE)(transposeA),
    (enum CBLAS_TRANSPOSE)(transposeB),
    m,
    n,
    k,
    alpha,
    a,
    lda,
    b,
    ldb,
    beta,
    c,
    ldc
  );
#endif

}

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
  int ldc) {

  naive_cblas_sgemm_fixed(
    order,
    transposeA,
    transposeB,
    m,
    n,
    k,
    alpha,
    a,
    aMin,
    aMax,
    aBitsPerElement,
    lda,
    b,
    ldb,
    beta,
    c,
    ldc
  );

}

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
  int ldc) {

  assert((transposeA == JPCblasNoTrans) || (transposeA == JPCblasTrans));
  assert(transposeB == JPCblasNoTrans);
  assert(order == JPCblasColMajor);

  int i, j, l;
  for (j = 0; j < n; j++) {
    for (i = 0; i < m; i++) {
      jpfloat_t total = 0.0f;
      for (l = 0; l < k; l++) {
        int aIndex;
        if (transposeA == JPCblasNoTrans) {
          aIndex = ((lda * l) + i);
        } else {
          aIndex = ((lda * i) + l);
        }
        const jpfloat_t aValue = a[aIndex];
        const int bIndex = ((ldb * j) + l);
        const jpfloat_t bValue = b[bIndex];
        total += (aValue * bValue);
      }
      const int cIndex = ((ldc * j) + i);
      const jpfloat_t oldCValue = c[cIndex];
      c[cIndex] = ((alpha * total) + (beta * oldCValue));
    }
  }
}

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
  int ldc) {

  assert((transposeA == JPCblasNoTrans) || (transposeA == JPCblasTrans));
  assert(transposeB == JPCblasNoTrans);
  assert(order == JPCblasColMajor);

  const jpfloat_t aRange = ((aMax - aMin) / (1 << aBitsPerElement));

  if (aBitsPerElement == 16) {
    uint16_t* aData = (uint16_t*)(a);
    int i, j, l;
    for (j = 0; j < n; j++) {
      for (i = 0; i < m; i++) {
        jpfloat_t total = 0.0f;
        for (l = 0; l < k; l++) {
          int aIndex;
          if (transposeA == JPCblasNoTrans) {
            aIndex = ((lda * l) + i);
          } else {
            aIndex = ((lda * i) + l);
          }
          const jpfloat_t aValue = aMin + (aData[aIndex] * aRange);
          const int bIndex = ((ldb * j) + l);
          const jpfloat_t bValue = b[bIndex];
          total += (aValue * bValue);
        }
        const int cIndex = ((ldc * j) + i);
        const jpfloat_t oldCValue = c[cIndex];
        c[cIndex] = ((alpha * total) + (beta * oldCValue));
      }
    }
  } else if (aBitsPerElement == 8) {
    uint8_t* aData = (uint8_t*)(a);
    int i, j, l;
    for (j = 0; j < n; j++) {
      for (i = 0; i < m; i++) {
        jpfloat_t total = 0.0f;
        for (l = 0; l < k; l++) {
          int aIndex;
          if (transposeA == JPCblasNoTrans) {
            aIndex = ((lda * l) + i);
          } else {
            aIndex = ((lda * i) + l);
          }
          const jpfloat_t aValue = aMin + (aData[aIndex] * aRange);
          const int bIndex = ((ldb * j) + l);
          const jpfloat_t bValue = b[bIndex];
          total += (aValue * bValue);
        }
        const int cIndex = ((ldc * j) + i);
        const jpfloat_t oldCValue = c[cIndex];
        c[cIndex] = ((alpha * total) + (beta * oldCValue));
      }
    }
  } else {
    assert(false); // Should never get here, only 8 or 16 bit supported
  }
}