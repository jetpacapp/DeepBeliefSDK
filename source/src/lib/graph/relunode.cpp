//
//  relunode.cpp
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#include "relunode.h"

#include <assert.h>
#include <string.h>

#include "buffer.h"
#include "binary_format.h"
#include "matrix_ops.h"

ReluNode::ReluNode() : BaseNode() {
  setClassName("ReluNode");
}

ReluNode::~ReluNode() {
  // Do nothing
}

Buffer* ReluNode::run(Buffer* input) {
  if (_output != NULL) {
    delete _output;
  }

  _output = matrix_max(input, 0.0f);

  return _output;
}

SBinaryTag* ReluNode::toTag() {
  SBinaryTag* resultDict = create_dict_tag();
  resultDict = add_string_to_dict(resultDict, "class", "relu");
  resultDict = add_string_to_dict(resultDict, "name", _name);
  return resultDict;
}

BaseNode* new_relunode_from_tag(SBinaryTag* tag, bool skipCopy) {
  const char* className = get_string_from_dict(tag, "class");
  assert(strcmp(className, "relu") == 0);
  ReluNode* result = new ReluNode();
  return result;
}
