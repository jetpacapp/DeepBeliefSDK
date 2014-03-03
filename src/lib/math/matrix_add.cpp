//
//  matrix_add.cpp
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#include "matrix_ops.h"

#include <assert.h>

#include "buffer.h"

void matrix_add_inplace(Buffer* output, Buffer* input, jpfloat_t inputScale) {
#ifdef DO_LOG_OPERATIONS
  fprintf(stderr, "matrix_add_inplace(output=[%s], input=[%s], inputScale=[%f])\n",
    output->debugString(),
    input->debugString(),
    inputScale);
#endif // DO_LOG_OPERATIONS
  assert((output->_dims.elementCount() % input->_dims.elementCount()) == 0);
  jpfloat_t* const outputDataStart = output->_data;
  jpfloat_t* const outputDataEnd = (outputDataStart + output->_dims.elementCount());
  const jpfloat_t* const inputDataStart = input->_data;
  const jpfloat_t* const inputDataEnd = (inputDataStart + input->_dims.elementCount());
  jpfloat_t* outputData = outputDataStart;
  const jpfloat_t* inputData = inputDataStart;
  while (outputData != outputDataEnd) {
    const jpfloat_t inputValue = *inputData;
    *outputData += (inputValue * inputScale);
    outputData += 1;
    inputData += 1;
    if (inputData >= inputDataEnd) {
      inputData = inputDataStart;
    }
  }
}
