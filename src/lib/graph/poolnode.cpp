//
//  poolnode.cpp
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#include "poolnode.h"

#include <assert.h>
#include <string.h>

#include "buffer.h"
#include "binary_format.h"
#include "matrix_ops.h"

PoolNode::PoolNode() : BaseNode() {
  setClassName("PoolNode");
}

PoolNode::~PoolNode() {
  // Do nothing
}

Buffer* PoolNode::run(Buffer* input) {
  if (_output != NULL) {
    delete _output;
  }
  _output = matrix_max_patch(input, _patchWidth, _stride);
  return _output;
}

BaseNode* new_poolnode_from_tag(SBinaryTag* tag, bool skipCopy) {
  const char* className = get_string_from_dict(tag, "class");
  assert(strcmp(className, "pool") == 0);
  PoolNode* result = new PoolNode();

  result->_patchWidth = get_uint_from_dict(tag, "psize");
  result->_stride = get_uint_from_dict(tag, "stride");

  const char* mode = get_string_from_dict(tag, "mode");
  if (strcmp(mode, "max") == 0) {
    result->_mode = PoolNode::EModeMax;
  } else if (strcmp(mode, "average") == 0) {
    result->_mode = PoolNode::EModeAverage;
  } else {
    fprintf(stderr, "Unknown pooling mode '%s'\n", mode);
  }

  return result;
}
