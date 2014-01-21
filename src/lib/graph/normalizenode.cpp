//
//  normalizenode.cpp
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#include "normalizenode.h"

#include <assert.h>
#include <string.h>

#include "buffer.h"
#include "binary_format.h"
#include "matrix_ops.h"

NormalizeNode::NormalizeNode() : BaseNode() {
  setClassName("NormalizeNode");
}

NormalizeNode::~NormalizeNode() {
  // Do nothing
}

Buffer* NormalizeNode::run(Buffer* input) {
  if (_output != NULL) {
    delete _output;
  }
  _output = matrix_local_response(input, _windowSize, _k, _alpha, _beta);
  return _output;
}

BaseNode* new_normalizenode_from_tag(SBinaryTag* tag, bool skipCopy) {
  const char* className = get_string_from_dict(tag, "class");
  assert(strcmp(className, "normalize") == 0);
  NormalizeNode* result = new NormalizeNode();

  result->_windowSize = get_uint_from_dict(tag, "size");
  result->_k = get_float_from_dict(tag, "k");
  result->_alpha = get_float_from_dict(tag, "alpha");
  result->_beta = get_float_from_dict(tag, "beta");

  return result;
}
