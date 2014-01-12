//
//  basenode.cpp
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#include "basenode.h"

#include "buffer.h"

BaseNode::BaseNode() {
  _output = NULL;
}

BaseNode::~BaseNode() {
  if (_output != NULL) {
    delete _output;
  }
}
