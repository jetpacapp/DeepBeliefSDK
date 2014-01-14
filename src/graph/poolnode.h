//
//  poolnode.h
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#ifndef INCLUDE_POOLNODE_H
#define INCLUDE_POOLNODE_H

#include "basenode.h"
#include "binary_format.h"

class Buffer;

class PoolNode : public BaseNode {
public:

  PoolNode();
  ~PoolNode();

  virtual Buffer* run(Buffer* input);

  int _patchWidth;
  int _stride;
  enum { EModeMax, EModeAverage } _mode;
};

BaseNode* new_poolnode_from_tag(SBinaryTag* tag);

#endif // INCLUDE_POOLNODE_H