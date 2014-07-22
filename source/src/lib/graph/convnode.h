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
  virtual SBinaryTag* toTag();
  virtual char* debugString();

  void saveDebugImage();

  uint32_t _kernelCount;
  uint32_t _kernelWidth;
  uint32_t _sampleStride;
  Buffer* _kernels;
  bool _useBias;
  Buffer* _bias;
  uint32_t _marginSize;
  bool _areKernelsTransposed;
};

BaseNode* new_convnode_from_tag(SBinaryTag* tag, bool skipCopy);

#endif // INCLUDE_CONVNODE_H