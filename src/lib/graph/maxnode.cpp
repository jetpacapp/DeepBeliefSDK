//
//  maxnode.cpp
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#include "maxnode.h"

#include <assert.h>
#include <string.h>

#include "buffer.h"
#include "binary_format.h"
#include "matrix_ops.h"

MaxNode::MaxNode() : BaseNode() {
  setClassName("MaxNode");
}

MaxNode::~MaxNode() {
  // Do nothing
}

Buffer* MaxNode::run(Buffer* input) {
  if (_output != NULL) {
    delete _output;
  }
  _output = matrix_softmax(input);
  return _output;
}

SBinaryTag* MaxNode::toTag() {
  SBinaryTag* resultDict = create_dict_tag();
  resultDict = add_string_to_dict(resultDict, "class", "max");
  resultDict = add_string_to_dict(resultDict, "name", _name);
  return resultDict;
}

BaseNode* new_maxnode_from_tag(SBinaryTag* tag, bool skipCopy) {
  const char* className = get_string_from_dict(tag, "class");
  assert(strcmp(className, "max") == 0);
  MaxNode* result = new MaxNode();
  return result;
}
