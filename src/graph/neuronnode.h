//
//  neuronnode.h
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#ifndef INCLUDE_NEURONNODE_H
#define INCLUDE_NEURONNODE_H

#include "basenode.h"
#include "binary_format.h"

class Buffer;

class NeuronNode : public BaseNode {
public:

  NeuronNode();
  ~NeuronNode();

  virtual Buffer* run(Buffer* input);
};

BaseNode* new_neuronnode_from_tag(SBinaryTag* tag);

#endif // INCLUDE_NEURONNODE_H