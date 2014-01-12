//
//  normalizenode.h
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#ifndef INCLUDE_NORMALIZENODE_H
#define INCLUDE_NORMALIZENODE_H

#include "basenode.h"
#include "binary_format.h"

class Buffer;

class NormalizeNode : public BaseNode {
public:

  NormalizeNode();
  ~NormalizeNode();

  virtual Buffer* run(Buffer* input);
};

BaseNode* new_normalizenode_from_tag(SBinaryTag* tag);

#endif // INCLUDE_NORMALIZENODE_H