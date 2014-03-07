//
//  gconvnode.h
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#ifndef INCLUDE_GCONVNODE_H
#define INCLUDE_GCONVNODE_H

#include "basenode.h"
#include "binary_format.h"

class Buffer;

class GConvNode : public BaseNode {
public:

  GConvNode();
  ~GConvNode();

  virtual Buffer* run(Buffer* input);
  virtual SBinaryTag* toTag();
  virtual char* debugString();

  int _subnodesCount;
  BaseNode** _subnodes;
  int _kernelsCount;
};

BaseNode* new_gconvnode_from_tag(SBinaryTag* tag, bool skipCopy);

#endif // INCLUDE_GCONVNODE_H