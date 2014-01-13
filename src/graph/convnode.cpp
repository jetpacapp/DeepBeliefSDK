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

#include "buffer.h"
#include "binary_format.h"

ConvNode::ConvNode() : BaseNode() {
  _kernels = NULL;
  setClassName("ConvNode");
}

ConvNode::~ConvNode() {
  if (_kernels != NULL) {
    delete _kernels;
  }
}

Buffer* ConvNode::run(Buffer* input) {
  return _output;
}

BaseNode* new_convnode_from_tag(SBinaryTag* tag) {
  const char* className = get_string_from_dict(tag, "class");
  assert(strcmp(className, "conv") == 0);
  ConvNode* result = new ConvNode();
  return result;
}
