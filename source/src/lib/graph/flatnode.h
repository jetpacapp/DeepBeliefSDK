//
//  flatnode.h
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#ifndef INCLUDE_FLATNODE_H
#define INCLUDE_FLATNODE_H

#include "basenode.h"
#include "binary_format.h"

class Buffer;

class FlatNode : public BaseNode {
public:

  FlatNode();
  ~FlatNode();

  virtual Buffer* run(Buffer* input);
  virtual SBinaryTag* toTag();
};

BaseNode* new_flatnode_from_tag(SBinaryTag* tag, bool skipCopy);

#endif // INCLUDE_FLATNODE_H