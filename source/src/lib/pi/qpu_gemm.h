//
//  qpu_gemm.h
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#ifndef INCLUDE_QPU_GEMM_H
#define INCLUDE_QPU_GEMM_H

#include "jpcnn.h"

void qpu_cblas_sgemm(
  int order,
  int transposeA,
  int transposeB,
  int m,
  int n,
  int k,
  jpfloat_t alpha,
  uint32_t a,
  int lda,
  uint32_t b,
  int ldb,
  jpfloat_t beta,
  uint32_t c,
  int ldc);

void qpu_cblas_sgemm_fixed(
  int order,
  int transposeA,
  int transposeB,
  int m,
  int n,
  int k,
  jpfloat_t alpha,
  uint32_t a,
  jpfloat_t aMin,
  jpfloat_t aMax,
  int aBitsPerElement,
  int lda,
  uint32_t b,
  int ldb,
  jpfloat_t beta,
  uint32_t c,
  int ldc);

void test_qpu_gemm();

#endif // INCLUDE_QPU_GEMM_H
