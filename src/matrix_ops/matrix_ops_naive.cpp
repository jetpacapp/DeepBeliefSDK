//
//  matrix_ops_naive.cpp
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#include "matrix_ops.h"

#include "assert.h"

#include "buffer.h"

void add_matrix_inplace(Buffer* output, Buffer* input, jpfloat_t inputScale) {
  assert(output->_dims == input->_dims);
  jpfloat_t* const outputDataStart = output->_data;
  jpfloat_t* const outputDataEnd = (outputDataStart + output->_dims.elementCount());
  const jpfloat_t* const inputDataStart = input->_data;
  jpfloat_t* outputData = outputDataStart;
  const jpfloat_t* inputData = inputDataStart;
  while (outputData != outputDataEnd) {
    const jpfloat_t inputValue = *inputData;
    *outputData += (inputValue * inputScale);
    outputData += 1;
    inputData += 1;
  }
}
