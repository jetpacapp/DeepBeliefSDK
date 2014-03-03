//
//  matrix_channels.cpp
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#include "matrix_ops.h"

#include <assert.h>
#include <string.h>

#include "buffer.h"

Buffer* matrix_extract_channels(Buffer* input, int startChannel, int endChannel) {
#ifdef DO_LOG_OPERATIONS
  fprintf(stderr, "matrix_extract_channels(input=[%s], startChannel=%d, endChannel=%d)\n",
    input->debugString(),
    startChannel,
    endChannel);
#endif // DO_LOG_OPERATIONS

  const Dimensions inputDims = input->_dims;
  const int inputChannels = inputDims._dims[inputDims._length - 1];
  const int outputChannels = (endChannel - startChannel);

  assert((inputChannels % outputChannels) == 0);

  Dimensions outputDims(inputDims);
  outputDims._dims[outputDims._length - 1] = outputChannels;
  Buffer* output = new Buffer(outputDims);

  const jpfloat_t* const inputDataStart = input->_data;
  const jpfloat_t* const inputDataEnd = (inputDataStart + inputDims.elementCount());

  jpfloat_t* const outputDataStart = output->_data;
  const size_t bytesInOutputRow = (outputChannels * sizeof(jpfloat_t));

  const jpfloat_t* inputData = (inputDataStart + startChannel);
  jpfloat_t* outputData = outputDataStart;
  while (inputData < inputDataEnd) {
    memcpy(outputData, inputData, bytesInOutputRow);
    inputData += inputChannels;
    outputData += outputChannels;
  }

#ifdef DO_LOG_OPERATIONS
  fprintf(stderr, "matrix_extract_channels() result=[%s]\n",
    output->debugString());
#endif // DO_LOG_OPERATIONS

  return output;
}

Buffer* matrix_join_channels(Buffer** inputs, int inputsCount) {
#ifdef DO_LOG_OPERATIONS
  char inputsDebugString[MAX_DEBUG_STRING_LEN] = "";
  for (int index = 0; index < inputsCount; index += 1) {
    const char* separator;
    if (index == (inputsCount - 1)) {
      separator = "";
    } else {
      separator = ", ";
    }
    snprintf(inputsDebugString, MAX_DEBUG_STRING_LEN, "%s(%s)%s",
      inputsDebugString, separator, inputs[index]->debugString());
  }
  fprintf(stderr, "matrix_join_channels(inputs=[%s], inputsCount=%d)\n",
    inputsDebugString, inputsCount);
#endif // DO_LOG_OPERATIONS

  Buffer* firstInput = inputs[0];
  const Dimensions inputDims = firstInput->_dims;
  const int inputChannels = inputDims._dims[inputDims._length - 1];
  const int outputChannels = (inputChannels * inputsCount);

  Dimensions outputDims(inputDims);
  outputDims._dims[outputDims._length - 1] = outputChannels;
  Buffer* output = new Buffer(outputDims);

  jpfloat_t** inputDatas = (jpfloat_t**)(malloc(sizeof(jpfloat_t*) * inputsCount));
  for (int index = 0; index < inputsCount; index += 1) {
    assert(inputs[index]->_dims == inputDims);
    inputDatas[index] = inputs[index]->_data;
  }
  const size_t bytesInInputRow = (inputChannels * sizeof(jpfloat_t));

  jpfloat_t* const outputDataStart = output->_data;
  jpfloat_t* const outputDataEnd = (outputDataStart + outputDims.elementCount());

  jpfloat_t* outputData = outputDataStart;
  while (outputData < outputDataEnd) {
    for (int index = 0; index < inputsCount; index += 1) {
      memcpy(outputData, inputDatas[index], bytesInInputRow);
      inputDatas[index] += inputChannels;
      outputData += inputChannels;
    }
  }

  free(inputDatas);

#ifdef DO_LOG_OPERATIONS
  fprintf(stderr, "matrix_join_channels() result=[%s]\n",
    output->debugString());
#endif // DO_LOG_OPERATIONS

  return output;
}
