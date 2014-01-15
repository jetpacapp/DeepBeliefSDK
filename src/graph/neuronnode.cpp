//
//  neuronnode.cpp
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#include "neuronnode.h"

#include <assert.h>
#include <string.h>

#include "buffer.h"
#include "binary_format.h"
#include "matrix_ops.h"

NeuronNode::NeuronNode() : BaseNode(), _weights(NULL), _bias(NULL) {
  setClassName("NeuronNode");
}

NeuronNode::~NeuronNode() {
  if (_weights != NULL) {
    delete _weights;
  }
  if (_bias != NULL) {
    delete _bias;
  }
}

Buffer* NeuronNode::run(Buffer* input) {

  Dimensions inputDims = input->_dims;
  const int inputChannels = inputDims[inputDims._length - 1];

  Dimensions expectedWeightsDimensions(inputChannels, _outputsCount);
  assert(expectedWeightsDimensions == _weights->_dims);

  _output = matrix_dot(input, _weights);
  _output->setName(_name);

  matrix_add_inplace(_output, _bias, 1.0);

  return _output;
}

BaseNode* new_neuronnode_from_tag(SBinaryTag* tag, bool skipCopy) {
  const char* className = get_string_from_dict(tag, "class");
  assert(strcmp(className, "neuron") == 0);
  NeuronNode* result = new NeuronNode();

  SBinaryTag* specDict = get_tag_from_dict(tag, "spec");
  result->_outputsCount = get_uint_from_dict(specDict, "num_output");

  SBinaryTag* weightsTag = get_tag_from_dict(tag, "weight");
  result->_weights = buffer_from_tag_dict(weightsTag, skipCopy);

  result->_useBias = (get_uint_from_dict(tag, "has_bias") != 0);
  if (result->_useBias) {
    SBinaryTag* biasTag = get_tag_from_dict(tag, "bias");
    result->_bias = buffer_from_tag_dict(biasTag, skipCopy);
  }
  
  return result;
}
