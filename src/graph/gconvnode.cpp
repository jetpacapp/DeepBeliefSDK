//
//  gconvnode.cpp
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#include "gconvnode.h"

#include <assert.h>
#include <string.h>

#include "buffer.h"
#include "binary_format.h"
#include "nodefactory.h"
#include "matrix_ops.h"

GConvNode::GConvNode() : BaseNode() {
  setClassName("GConvNode");
}

GConvNode::~GConvNode() {
  if (_subnodes != NULL) {
    for (int index = 0; index < _subnodesCount; index += 1) {
      delete _subnodes[index];
    }
    free(_subnodes);
  }
}

Buffer* GConvNode::run(Buffer* input) {
  if (_output != NULL) {
    delete _output;
  }

  const Dimensions inputDims = input->_dims;

  assert(inputDims._length == 4);

  const int imageCount = inputDims[0];
  const int inputWidth = inputDims[2];
  const int inputHeight = inputDims[1];
  const int inputChannels = inputDims[3];

  assert((inputChannels % _subnodesCount) == 0);
  const int subnodeChannels = (inputChannels / _subnodesCount);

  const Dimensions subnodeInputDimensions(imageCount, inputHeight, inputWidth, subnodeChannels);
  Buffer** subnodeOutputBuffers = (Buffer**)(malloc(sizeof(Buffer*) * _subnodesCount));

  for (int index = 0; index < _subnodesCount; index += 1) {
    const int startChannel = (index * subnodeChannels);
    const int endChannel = ((index + 1) * subnodeChannels);
    Buffer* subnodeInputBuffer = matrix_extract_channels(input, startChannel, endChannel);

    BaseNode* subnode = _subnodes[index];
    Buffer* subnodeOutputBuffer = subnode->run(subnodeInputBuffer);
    subnodeOutputBuffers[index] = subnodeOutputBuffer;

    delete subnodeInputBuffer;
  }

  _output = matrix_join_channels(subnodeOutputBuffers, _subnodesCount);

  return _output;
}

BaseNode* new_gconvnode_from_tag(SBinaryTag* tag, bool skipCopy) {
  const char* className = get_string_from_dict(tag, "class");
  assert(strcmp(className, "gconv") == 0);
  GConvNode* result = new GConvNode();

  result->_subnodesCount = get_uint_from_dict(tag, "layers_count");
  result->_subnodes = (BaseNode**)(malloc(sizeof(BaseNode*) * result->_subnodesCount));

  SBinaryTag* subnodesTag = get_tag_from_dict(tag, "layers");

  int index = 0;
  SBinaryTag* currentSubnodeTag = get_first_list_entry(subnodesTag);
  while (currentSubnodeTag != NULL) {
    BaseNode* subnode = new_node_from_tag(currentSubnodeTag, skipCopy);
    result->_subnodes[index] = subnode;
    index += 1;
    currentSubnodeTag = get_next_list_entry(subnodesTag, currentSubnodeTag);
  }

  result->_kernelsCount = get_uint_from_dict(tag, "kernels_count");

  return result;
}
