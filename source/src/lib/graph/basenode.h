//
//  basenode.h
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#ifndef INCLUDE_BASENODE_H
#define INCLUDE_BASENODE_H

#include "jpcnn.h"
#include "binary_format.h"

class Buffer;

class BaseNode
{
public:

  BaseNode();
  virtual ~BaseNode();

  virtual Buffer* run(Buffer* input) = 0;
  virtual SBinaryTag* toTag() = 0;

  void setClassName(const char* name);
  void setName(const char* name);
  virtual char* debugString();
  virtual char* debugStringWithMessage(const char* subclassMessage);

  Buffer* _output;
  char* _className;
  char* _name;
  char* _debugString;
};

#endif // INCLUDE_BASENODE_H