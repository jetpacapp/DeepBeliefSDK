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

ReluNode::ReluNode() : BaseNode() {
}

ReluNode::~ReluNode() {
  // Do nothing
}

Buffer* ReluNode::run(Buffer* input) {
  return input;
}

BaseNode* new_relunode_from_tag(SBinaryTag* tag) {
  const char* name = get_string_from_dict(tag, "name");
  assert(strcmp(name, "relu") == 0);
  ReluNode* result = new ReluNode();
  return result;
}
