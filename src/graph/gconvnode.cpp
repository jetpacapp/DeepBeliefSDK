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

GConvNode::GConvNode() : BaseNode() {
}

GConvNode::~GConvNode() {
  // Do nothing
}

Buffer* GConvNode::run(Buffer* input) {
  return input;
}

BaseNode* new_gconvnode_from_tag(SBinaryTag* tag) {
  const char* name = get_string_from_dict(tag, "name");
  assert(strcmp(name, "gconv") == 0);
  GConvNode* result = new GConvNode();
  return result;
}
