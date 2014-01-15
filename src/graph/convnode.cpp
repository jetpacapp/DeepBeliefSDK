//
//  convnode.cpp
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#include "convnode.h"

#include <assert.h>
#include <string.h>
#include <math.h>

#include "buffer.h"
#include "binary_format.h"
#include "matrix_ops.h"

ConvNode::ConvNode() : BaseNode(), _kernels(NULL), _bias(NULL) {
  setClassName("ConvNode");
}

ConvNode::~ConvNode() {
  if (_kernels != NULL) {
    delete _kernels;
  }
  if (_bias != NULL) {
    delete _bias;
  }
}

Buffer* ConvNode::run(Buffer* input) {

  Dimensions inputDims = input->_dims;
  const int inputChannels = inputDims[inputDims._length - 1];
  const int valuesPerKernel = (inputChannels * _kernelWidth * _kernelWidth);
  Dimensions expectedKernelsDims(valuesPerKernel, _kernelCount);
  assert(expectedKernelsDims == _kernels->_dims);

  Buffer* inputWithMargin;
  if (_marginSize == 0) {
    inputWithMargin = input;
  } else {
    inputWithMargin = matrix_insert_margin(input, _marginSize, _marginSize);
  }

  _output = matrix_correlate(inputWithMargin, _kernels, _kernelWidth, _kernelCount, _sampleStride);
  _output->setName(_name);

  matrix_add_inplace(_output, _bias, 1.0);

  if (_marginSize != 0) {
    delete inputWithMargin;
  }

  return _output;
}

BaseNode* new_convnode_from_tag(SBinaryTag* tag, bool skipCopy) {
  const char* className = get_string_from_dict(tag, "class");
  assert(strcmp(className, "conv") == 0);
  ConvNode* result = new ConvNode();

  SBinaryTag* specDict = get_tag_from_dict(tag, "spec");
  result->_kernelCount = get_uint_from_dict(specDict, "num_kernels");
  result->_kernelWidth = get_uint_from_dict(specDict, "ksize");
  result->_sampleStride = get_uint_from_dict(specDict, "stride");

  SBinaryTag* kernelsTag = get_tag_from_dict(tag, "kernels");
  result->_kernels = buffer_from_tag_dict(kernelsTag, skipCopy);

  result->_useBias = (get_uint_from_dict(tag, "has_bias") != 0);
  if (result->_useBias) {
    SBinaryTag* biasTag = get_tag_from_dict(tag, "bias");
    result->_bias = buffer_from_tag_dict(biasTag, skipCopy);
  }

  result->_marginSize = get_uint_from_dict(tag, "padding");

  return result;
}
