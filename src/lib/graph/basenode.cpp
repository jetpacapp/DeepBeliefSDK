//
//  basenode.cpp
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#include "basenode.h"

#include <stdlib.h>
#include <string.h>

#include "buffer.h"

BaseNode::BaseNode() : _output(NULL), _className(NULL), _name(NULL) {
  _output = NULL;
}

BaseNode::~BaseNode() {
  if (_output != NULL) {
    delete _output;
  }
  if (_className != NULL) {
    free(_className);
  }
  if (_name != NULL) {
    free(_name);
  }
}

void BaseNode::setClassName(const char* className) {
  const size_t length = strlen(className) + 1;
  _className = (char*)(malloc(length));
  strncpy(_className, className, length);
}

void BaseNode::setName(const char* name) {
  const size_t length = strlen(name) + 1;
  _name = (char*)(malloc(length));
  strncpy(_name, name, length);
}

