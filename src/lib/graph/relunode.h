//
//  relunode.h
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#ifndef INCLUDE_RELUNODE_H
#define INCLUDE_RELUNODE_H

#include "basenode.h"
#include "binary_format.h"

class Buffer;

class ReluNode : public BaseNode {
public:

  ReluNode();
  ~ReluNode();

  virtual Buffer* run(Buffer* input);
  virtual SBinaryTag* toTag();
};

BaseNode* new_relunode_from_tag(SBinaryTag* tag, bool skipCopy);

#endif // INCLUDE_RELUNODE_H