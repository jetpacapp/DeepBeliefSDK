//
//  prepareinput.h
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#ifndef INCLUDE_PREPAREINPUT_H
#define INCLUDE_PREPAREINPUT_H

#include "jpcnn.h"

#include "basenode.h"

class PrepareInput : BaseNode {
public:

  PrepareInput(Buffer* dataMean, bool useCenterOnly, bool needsFlip, bool doRandomSample, int imageSize, int rescaledSize, bool isMeanChanneled);
  ~PrepareInput();

  virtual Buffer* run(Buffer* input);
  virtual SBinaryTag* toTag();

  Buffer* _dataMean;
  bool _useCenterOnly;
  bool _needsFlip;
  bool _doRandomSample;
  const int _imageSize;
  const int _rescaledSize;
};

#endif // INCLUDE_PREPAREINPUT_H

