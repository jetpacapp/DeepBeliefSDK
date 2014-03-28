//
//  glbuffer.h
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#ifndef INCLUDE_GLBUFFER_H
#define INCLUDE_GLBUFFER_H

#include "jpcnn.h"

#include "glheaders.h"

class GLContext;
class Dimensions;
class Buffer;

class GLBuffer {
public:
  GLBuffer(GLContext* context, const Dimensions& dims);
  GLBuffer(GLContext* context, const Dimensions& dims, jpfloat_t* data);
  GLBuffer(GLContext* context, const Dimensions& dims, void* data, jpfloat_t min, jpfloat_t max, int bitsPerElement);
  virtual ~GLBuffer();

  void copyHostBufferToGPU();

  GLContext* _context;
  Buffer* _hostBuffer;
  GLuint _textureId;
};

#endif // INCLUDE_GLBUFFER_H
