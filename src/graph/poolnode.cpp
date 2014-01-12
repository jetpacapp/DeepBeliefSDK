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

PoolNode::PoolNode() : BaseNode() {
}

PoolNode::~PoolNode() {
  // Do nothing
}

Buffer* PoolNode::run(Buffer* input) {
  return input;
}

BaseNode* new_poolnode_from_tag(SBinaryTag* tag) {
  const char* name = get_string_from_dict(tag, "name");
  assert(strcmp(name, "pool") == 0);
  PoolNode* result = new PoolNode();
  return result;
}
