//
//  flatnode.cpp
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#include "flatnode.h"

#include <assert.h>
#include <string.h>

#include "buffer.h"
#include "binary_format.h"

FlatNode::FlatNode() : BaseNode() {
  setClassName("FlatNode");
}

FlatNode::~FlatNode() {
  // Do nothing
}

Buffer* FlatNode::run(Buffer* input) {
  const Dimensions inputDims = input->_dims;
  // We're expecting (# of images, height, width, # of channels)
  assert(inputDims._length == 4);

  const int imageCount = inputDims[0];
  const int inputWidth = inputDims[2];
  const int inputHeight = inputDims[1];
  const int inputChannels = inputDims[3];

  const int outputElementCount = (inputHeight * inputWidth * inputChannels);
  const Dimensions outputDims(imageCount, outputElementCount);

  // Doesn't do a data copy, just returns a new view with a different shape.
  _output = new Buffer(outputDims, input->_data);

  return _output;
}

SBinaryTag* FlatNode::toTag() {
  SBinaryTag* resultDict = create_dict_tag();
  resultDict = add_string_to_dict(resultDict, "class", "flat");
  resultDict = add_string_to_dict(resultDict, "name", _name);

  return resultDict;
}

BaseNode* new_flatnode_from_tag(SBinaryTag* tag, bool skipCopy) {
  const char* className = get_string_from_dict(tag, "class");
  assert(strcmp(className, "flat") == 0);
  FlatNode* result = new FlatNode();
  return result;
}
