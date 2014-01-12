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

class Buffer;

class BaseNode
{
public:

  BaseNode();
  virtual ~BaseNode();

  virtual Buffer* run(Buffer* input)=0;

  Buffer* _output;
};

#endif // INCLUDE_BASENODE_H