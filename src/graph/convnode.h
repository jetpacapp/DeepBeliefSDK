//
//  convnode.h
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#ifndef INCLUDE_CONVNODE_H
#define INCLUDE_CONVNODE_H

#include "basenode.h"
#include "binary_format.h"

class Buffer;

class ConvNode : public BaseNode {
public:

  ConvNode();
  ~ConvNode();

  virtual Buffer* run(Buffer* input);

  Buffer* _kernels;
};

BaseNode* new_convnode_from_tag(SBinaryTag* tag);

#endif // INCLUDE_CONVNODE_H