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

NeuronNode::NeuronNode() : BaseNode(), _weights(NULL), _bias(NULL), _dropout(0.0f), _areWeightsTransposed(false) {
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
  if (_output != NULL) {
    delete _output;
  }

  const Dimensions inputDims = input->_dims;
  const int numberOfImages = inputDims[0];
  const Dimensions inputImageDims = inputDims.removeDimensions(1);
  const int elementCount = inputImageDims.elementCount();
  Dimensions flattenedDimensions(numberOfImages, elementCount);
  Buffer* flattenedInput = input->view();
  flattenedInput->reshape(flattenedDimensions);

  if (_areWeightsTransposed) {
    Dimensions expectedWeightsDimensions(_outputsCount, elementCount);
    assert(expectedWeightsDimensions == _weights->_dims);
  } else {
    Dimensions expectedWeightsDimensions(elementCount, _outputsCount);
    assert(expectedWeightsDimensions == _weights->_dims);
  }

//_weights->quantize(8);

  _output = matrix_dot(flattenedInput, _weights, _areWeightsTransposed);
  _output->setName(_name);

  matrix_add_inplace(_output, _bias, 1.0);

  if (_dropout > 0.0f) {
    const float scale = (1.0f - _dropout);
    matrix_scale_inplace(_output, scale);
  }

  delete flattenedInput;

  return _output;
}

char* NeuronNode::debugString() {
  char additionalInfo[MAX_DEBUG_STRING_LEN];
  snprintf(additionalInfo, sizeof(additionalInfo),
    "_outputsCount=%d, _useBias=%d, _weights->_dims=%s",
    _outputsCount, _useBias, _weights->_dims.debugString());
  return this->debugStringWithMessage(additionalInfo);
}

SBinaryTag* NeuronNode::toTag() {
  SBinaryTag* resultDict = create_dict_tag();
  resultDict = add_string_to_dict(resultDict, "class", "neuron");
  resultDict = add_string_to_dict(resultDict, "name", _name);

  SBinaryTag* specDict = create_dict_tag();
  specDict = add_uint_to_dict(specDict, "num_output", _outputsCount);
  resultDict = add_tag_to_dict(resultDict, "spec", specDict);
  free(specDict);

  const bool wantTransposedOutput = true;
  const int outputBitDepth = 8;

  if (wantTransposedOutput != _areWeightsTransposed) {
    _weights->transpose(); // First transpose so they match
  }
  SBinaryTag* weightsTag = buffer_to_tag_dict(_weights, outputBitDepth);
  resultDict = add_tag_to_dict(resultDict, "weight", weightsTag);
  free(weightsTag);
  if (wantTransposedOutput != _areWeightsTransposed) {
    _weights->transpose(); // Undo the original transpose by applying another
  }

  if (wantTransposedOutput) {
    resultDict = add_uint_to_dict(resultDict, "are_weights_transposed", 1);
  } else {
    resultDict = add_uint_to_dict(resultDict, "are_weights_transposed", 0);
  }

  resultDict = add_uint_to_dict(resultDict, "has_bias", _useBias);
  if (_useBias) {
    SBinaryTag* biasTag = buffer_to_tag_dict(_bias);
    resultDict = add_tag_to_dict(resultDict, "bias", biasTag);
    free(biasTag);
  }

  resultDict = add_float_to_dict(resultDict, "dropout", _dropout);

  return resultDict;
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

  if (get_tag_from_dict(tag, "dropout") != NULL) {
    result->_dropout = get_float_from_dict(tag, "dropout");
  }

  if (get_tag_from_dict(tag, "are_weights_transposed")) {
    result->_areWeightsTransposed = get_uint_from_dict(tag, "are_weights_transposed");
  }

  return result;
}
