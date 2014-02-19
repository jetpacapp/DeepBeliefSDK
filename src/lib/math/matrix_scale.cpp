//
//  matrix_scale.cpp
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#include "matrix_ops.h"

#include <assert.h>

#include "buffer.h"

void matrix_scale_inplace(Buffer* output, jpfloat_t scale) {
  jpfloat_t* const outputDataStart = output->_data;
  jpfloat_t* const outputDataEnd = (outputDataStart + output->_dims.elementCount());
  jpfloat_t* outputData = outputDataStart;
  while (outputData != outputDataEnd) {
    const jpfloat_t inputValue = *outputData;
    *outputData = (inputValue * scale);
    outputData += 1;
  }
}
