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

NormalizeNode::NormalizeNode() : BaseNode() {
  setClassName("NormalizeNode");
}

NormalizeNode::~NormalizeNode() {
  // Do nothing
}

Buffer* NormalizeNode::run(Buffer* input) {
  return input;
}

BaseNode* new_normalizenode_from_tag(SBinaryTag* tag) {
  const char* className = get_string_from_dict(tag, "class");
  assert(strcmp(className, "normalize") == 0);
  NormalizeNode* result = new NormalizeNode();
  return result;
}
