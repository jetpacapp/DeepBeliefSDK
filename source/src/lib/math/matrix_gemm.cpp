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
#include <stdint.h>
#include <stdlib.h>

#ifdef USE_ACCELERATE_GEMM
#include <Accelerate/Accelerate.h>
#endif

#ifdef USE_MKL_GEMM
#include <mkl_cblas.h>
#endif // USE_MKL_GEMM

#ifdef USE_ATLAS_GEMM
#include <cblas.h>
#endif // USE_ATLAS_GEMM

#ifdef USE_EIGEN_GEMM
#include <Eigen/Dense>
#endif // USE_EIGEN_GEMM

#ifdef USE_OPENGL
#include "glgemm.h"
#endif // USE_OPENGL

#ifdef USE_NEON
#include <arm_neon.h>
#include <omp.h>
#endif // USE_NEON

#if !defined(USE_ACCELERATE_GEMM) && !defined(USE_MKL_GEMM) && !defined(USE_OPENGL) && !defined(USE_ATLAS_GEMM) && !defined(USE_EIGEN_GEMM) && !defined(USE_QPU_GEMM)
#define USE_NAIVE_GEMM
#endif

#define DO_LOG_OPERATIONS

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
    (CBLAS_ORDER)(order),
    (CBLAS_TRANSPOSE)(transposeA),
    (CBLAS_TRANSPOSE)(transposeB),
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
#elif defined(USE_EIGEN_GEMM)
  eigen_cblas_sgemm(
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
#elif defined(USE_QPU_GEMM)
  assert(false); // You need to call the GEMM function directly so it has access to the GPU memory
#else
#error "No GEMM implementation defined"
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

#if defined(USE_OPENGL)
  gl_gemm_fixed(
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
#elif defined(USE_ACCELERATE_GEMM) || defined(USE_MKL_GEMM) || defined(USE_ATLAS_GEMM) || defined(USE_EIGEN_GEMM)
  cblas_sgemm_fixed(
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
#elif defined(USE_QPU_GEMM)
  assert(false); // You need to call the GEMM function directly so it has access to the GPU memory
#else
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
#endif
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
          uint16_t aIntValue = aData[aIndex];
          const jpfloat_t aValue = aMin + (aIntValue * aRange);
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
          uint8_t aIntValue = aData[aIndex];
          const jpfloat_t aValue = aMin + (aIntValue * aRange);
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

#if defined(USE_ACCELERATE_GEMM) || defined(USE_MKL_GEMM) || defined(USE_ATLAS_GEMM) || defined(USE_EIGEN_GEMM)
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
  int ldc) {

  assert(transposeA == JPCblasTrans);
  assert(transposeB == JPCblasNoTrans);
  assert(order == JPCblasColMajor);

  jpfloat_t aRange = ((aMax - aMin) / (1 << aBitsPerElement));

  const int rowsPerOperation = 64;

  const size_t bytesPerRow = (k * sizeof(jpfloat_t));
  const size_t bytesPerSubMatrix = (bytesPerRow * rowsPerOperation);
#if defined(TARGET_PI)
  jpfloat_t* aSubMatrix;
  posix_memalign((void**)(&aSubMatrix), 16, bytesPerSubMatrix);
  jpfloat_t* cSubMatrix;
  posix_memalign((void**)(&cSubMatrix), 16, bytesPerSubMatrix);
#elif defined(USE_EIGEN_GEMM)
  jpfloat_t* aSubMatrix = (jpfloat_t*)(Eigen::internal::aligned_malloc(bytesPerSubMatrix));
  jpfloat_t* cSubMatrix = (jpfloat_t*)(Eigen::internal::aligned_malloc(bytesPerSubMatrix));
#else // TARGET_PI
  jpfloat_t* aSubMatrix = (jpfloat_t*)(malloc(bytesPerSubMatrix));
  jpfloat_t* cSubMatrix = (jpfloat_t*)(malloc(bytesPerSubMatrix));
#endif // TARGET_PI

  for (int iBase = 0; iBase < m; iBase += rowsPerOperation) {
    const int rowsThisTime = MIN(rowsPerOperation, (m - iBase));
    const int elementsPerSubMatrix = (rowsThisTime * k);
    if (aBitsPerElement == 16) {
      uint16_t* aData = (uint16_t*)(a);
#ifdef USE_ACCELERATE_GEMM
      uint16_t* aSubDataStart = (aData + (lda * iBase));
      vDSP_vfltu16(
        aSubDataStart,
        1,
        aSubMatrix,
        1,
        elementsPerSubMatrix);
      vDSP_vsmsa(
        aSubMatrix,
        1,
        &aRange,
        &aMin,
        aSubMatrix,
        1,
        elementsPerSubMatrix
      );
#elif defined(USE_NEON)

      // Only works on data that's multiples of 8 in size
      assert((k % 8) == 0);

      const float32x4_t vAMin0 = vdupq_n_f32(aMin);
      const float32x4_t vAMin1 = vdupq_n_f32(aMin);
      const float32x4_t vARange0 = vdupq_n_f32(aRange);
      const float32x4_t vARange1 = vdupq_n_f32(aRange);

      for (int iOffset = 0; iOffset < rowsThisTime; iOffset += 1) {
        const int i = (iBase + iOffset);
        uint16_t* currentA = (aData + (lda * i));
        uint16_t* endA = (currentA + k);
        jpfloat_t* currentSubMatrix = (aSubMatrix + (lda * iOffset));
        while (currentA < endA) {
          uint16x8_t vAInput16Bit = vld1q_u16(currentA);
          uint16x4_t vAInput16BitHigh = vget_high_u16(vAInput16Bit);
          uint16x4_t vAInput16BitLow = vget_low_u16(vAInput16Bit);
          uint32x4_t vAInput32BitHigh = vmovl_u16(vAInput16BitHigh);
          uint32x4_t vAInput32BitLow = vmovl_u16(vAInput16BitLow);
          float32x4_t vAInputFloatHigh = vcvtq_f32_u32(vAInput32BitHigh);
          float32x4_t vAInputFloatLow = vcvtq_f32_u32(vAInput32BitLow);
          float32x4_t vOutputHigh = vmlaq_f32(vAMin0, vARange0, vAInputFloatHigh);
          float32x4_t vOutputLow = vmlaq_f32(vAMin1, vARange1, vAInputFloatLow);

          vst1q_f32(currentSubMatrix, vOutputLow);
          currentSubMatrix += 4;
          vst1q_f32(currentSubMatrix, vOutputHigh);
          currentSubMatrix += 4;

          currentA += 8;

          const uint16_t dummyValue = *currentA;
        }
      }
#else // USE_ACCELERATE_GEMM
      for (int iOffset = 0; iOffset < rowsThisTime; iOffset += 1) {
        const int i = (iBase + iOffset);
        uint16_t* currentA = (aData + (lda * i));
        uint16_t* endA = (currentA + k);
        jpfloat_t* currentSubMatrix = (aSubMatrix + (lda * iOffset));
        while (currentA < endA) {
          *currentSubMatrix = (aMin + ((*currentA) * aRange));
          currentA += 1;
          currentSubMatrix += 1;
        }
      }
#endif // USE_ACCELERATE_GEMM
    } else if (aBitsPerElement == 8) {
      uint8_t* aData = (uint8_t*)(a);
#ifdef USE_ACCELERATE_GEMM
      uint8_t* aSubDataStart = (aData + (lda * iBase));
      vDSP_vfltu8(
        aSubDataStart,
        1,
        aSubMatrix,
        1,
        elementsPerSubMatrix);
      vDSP_vsmsa(
        aSubMatrix,
        1,
        &aRange,
        &aMin,
        aSubMatrix,
        1,
        elementsPerSubMatrix
      );
#elif defined(USE_NEON)

      // Only works on data that's multiples of 8 in size
      assert((k % 8) == 0);

      const float32x4_t vAMin0 = vdupq_n_f32(aMin);
      const float32x4_t vAMin1 = vdupq_n_f32(aMin);
      const float32x4_t vARange0 = vdupq_n_f32(aRange);
      const float32x4_t vARange1 = vdupq_n_f32(aRange);

      for (int iOffset = 0; iOffset < rowsThisTime; iOffset += 1) {
        const int i = (iBase + iOffset);
        uint8_t* currentA = (aData + (lda * i));
        uint8_t* endA = (currentA + k);
        jpfloat_t* currentSubMatrix = (aSubMatrix + (lda * iOffset));
        while (currentA < endA) {
          uint8x8_t vAInput8Bit = vld1_u8(currentA);
          uint16x8_t vAInput16Bit = vmovl_u8(vAInput8Bit);
          uint16x4_t vAInput16BitHigh = vget_high_u16(vAInput16Bit);
          uint16x4_t vAInput16BitLow = vget_low_u16(vAInput16Bit);
          uint32x4_t vAInput32BitHigh = vmovl_u16(vAInput16BitHigh);
          uint32x4_t vAInput32BitLow = vmovl_u16(vAInput16BitLow);
          float32x4_t vAInputFloatHigh = vcvtq_f32_u32(vAInput32BitHigh);
          float32x4_t vAInputFloatLow = vcvtq_f32_u32(vAInput32BitLow);
          float32x4_t vOutputHigh = vmlaq_f32(vAMin0, vARange0, vAInputFloatHigh);
          float32x4_t vOutputLow = vmlaq_f32(vAMin1, vARange1, vAInputFloatLow);

          vst1q_f32(currentSubMatrix, vOutputLow);
          currentSubMatrix += 4;
          vst1q_f32(currentSubMatrix, vOutputHigh);
          currentSubMatrix += 4;

          currentA += 8;

          const uint8_t dummyValue = *currentA;
        }
      }
#else // USE_ACCELERATE_GEMM
      for (int iOffset = 0; iOffset < rowsThisTime; iOffset += 1) {
        const int i = (iBase + iOffset);
        uint8_t* currentA = (aData + (lda * i));
        uint8_t* endA = (currentA + k);
        jpfloat_t* currentSubMatrix = (aSubMatrix + (lda * iOffset));
        while (currentA < endA) {
          *currentSubMatrix = (aMin + ((*currentA) * aRange));
          currentA += 1;
          currentSubMatrix += 1;
        }
      }
#endif // USE_ACCELERATE_GEMM
    } else {
      assert(false); // Should never get here, only 8 or 16 bit supported
    }
#if defined(USE_EIGEN_GEMM)
    eigen_cblas_sgemm(
      order,
      transposeA,
      transposeB,
      rowsThisTime,
      n,
      k,
      alpha,
      aSubMatrix,
      lda,
      b,
      ldb,
      beta,
      (c + iBase),
      ldc
    );
#else // USE_EIGEN_GEMM
    cblas_sgemm(
      (CBLAS_ORDER)(order),
      (CBLAS_TRANSPOSE)(transposeA),
      (CBLAS_TRANSPOSE)(transposeB),
      rowsThisTime,
      n,
      k,
      alpha,
      aSubMatrix,
      lda,
      b,
      ldb,
      beta,
      (c + iBase),
      ldc
    );
#endif // USE_EIGEN_GEMM
  }

#if defined(USE_EIGEN_GEMM)
  Eigen::internal::aligned_free(aSubMatrix);
  Eigen::internal::aligned_free(cSubMatrix);
#else // USE_EIGEN_GEMM
  free(aSubMatrix);
  free(cSubMatrix);
#endif // USE_EIGEN_GEMM
}
#endif

#if defined(USE_EIGEN_GEMM)

typedef Eigen::Stride<Eigen::Dynamic, Eigen::Dynamic> DynamicStride;
typedef Eigen::Matrix<jpfloat_t, Eigen::Dynamic, Eigen::Dynamic, Eigen::ColMajor> EigenMatrix;
typedef Eigen::Map<EigenMatrix, Eigen::Aligned, DynamicStride> eigen_matrix_t;
typedef Eigen::Map<const EigenMatrix, Eigen::Aligned> eigen_matrix_const_t;

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
  int ldc) {

  assert((transposeA == JPCblasNoTrans) || (transposeA == JPCblasTrans));
  assert(transposeB == JPCblasNoTrans);
  assert(order == JPCblasColMajor);
  assert(alpha == 1.0f);

  eigen_matrix_t cMatrix(c, m, n, DynamicStride(ldc, 1));
  if (beta > 0.0f) {
    cMatrix *= beta;
  }
  eigen_matrix_const_t bMatrix(b, k, n);
  if (transposeA == JPCblasNoTrans) {
    eigen_matrix_const_t aMatrix(a, m, k);
    if (beta > 0.0f) {
      cMatrix.noalias() += (aMatrix * bMatrix);
    } else {
      cMatrix.noalias() = (aMatrix * bMatrix);
    }
  } else {
    eigen_matrix_const_t aMatrix(a, k, m);
    if (beta > 0.0f) {
      cMatrix.noalias() += (aMatrix.transpose() * bMatrix);
    } else {
      cMatrix.noalias() = (aMatrix.transpose() * bMatrix);
    }
  }
}
#endif // USE_EIGEN_GEMM
