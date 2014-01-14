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
Buffer* matrix_correlate(Buffer* input, Buffer* kernels, int kernelWidth, int kernelCount, int stride);
Buffer* matrix_dot(Buffer* a, Buffer* b);
Buffer* matrix_extract_channels(Buffer* input, int startChannel, int endChannel);
Buffer* matrix_insert_margin(Buffer* input, int marginWidth, int marginHeight);
Buffer* matrix_join_channels(Buffer** inputs, int inputsCount);
Buffer* matrix_local_response(Buffer* input, int windowSize, jpfloat_t k, jpfloat_t alpha, jpfloat_t beta);
Buffer* matrix_max(Buffer* input, jpfloat_t maxValue);
Buffer* matrix_max_patch(Buffer* input, int patchWidth, int stride);

#endif // INCLUDE_MATRIX_OPS_H
