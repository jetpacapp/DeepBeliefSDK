//
//  dropoutnode.h
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#ifndef INCLUDE_DROPOUTNODE_H
#define INCLUDE_DROPOUTNODE_H

#include "basenode.h"
#include "binary_format.h"

class Buffer;

class DropoutNode : public BaseNode {
public:

  DropoutNode();
  ~DropoutNode();

  virtual Buffer* run(Buffer* input);
  virtual SBinaryTag* toTag();
};

BaseNode* new_dropoutnode_from_tag(SBinaryTag* tag, bool skipCopy);

#endif // INCLUDE_DROPOUTNODE_H