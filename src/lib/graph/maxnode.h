//
//  maxnode.h
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#ifndef INCLUDE_MAXNODE_H
#define INCLUDE_MAXNODE_H

#include "basenode.h"
#include "binary_format.h"

class Buffer;

class MaxNode : public BaseNode {
public:

  MaxNode();
  ~MaxNode();

  virtual Buffer* run(Buffer* input);
  virtual SBinaryTag* toTag();
};

BaseNode* new_maxnode_from_tag(SBinaryTag* tag, bool skipCopy);

#endif // INCLUDE_MAXNODE_H