//
//  glbuffer.cpp
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#ifdef USE_OPENGL

#include "glbuffer.h"

#include <assert.h>

#include "glheaders.h"

#include "buffer.h"
#include "dimensions.h"
#include "glcontext.h"

GLBuffer::GLBuffer(GLContext* context, const Dimensions& dims) {
  // We expect (height, width, channels)
  assert(dims._length == 3);
  const int channels = dims[2];
  assert((channels == 1) || (channels == 3) || (channels == 4));
  _context = context;
  _hostBuffer = new Buffer(dims, NULL);
  copyHostBufferToGPU();
}

GLBuffer::GLBuffer(GLContext* context, const Dimensions& dims, jpfloat_t* data) {
  // We expect (height, width, channels)
  assert(dims._length == 3);
  const int channels = dims[2];
  assert((channels == 1) || (channels == 3) || (channels == 4));
  _context = context;
  _hostBuffer = new Buffer(dims, data);
  copyHostBufferToGPU();
}

GLBuffer::GLBuffer(GLContext* context, const Dimensions& dims, void* data, jpfloat_t min, jpfloat_t max, int bitsPerElement) {
  // We expect (height, width, channels)
  assert(dims._length == 3);
  const int channels = dims[2];
  assert((channels == 1) || (channels == 3) || (channels == 4));
  _context = context;
  _hostBuffer = new Buffer(dims, data, min, max, bitsPerElement);
  copyHostBufferToGPU();
}

GLBuffer::~GLBuffer() {
  delete _hostBuffer;
  glDeleteTextures(1, &_textureId);
}

void GLBuffer::copyHostBufferToGPU() {
  glGenTextures(1, &_textureId);
  glBindTexture(GL_TEXTURE_2D, _textureId);

  Buffer* hostBuffer = _hostBuffer;
  const Dimensions& bufferDims = hostBuffer->_dims;
  const int width = bufferDims[1];
  const int height = bufferDims[0];
  const int channels = bufferDims[2];
  const int bitsPerElement = hostBuffer->_bitsPerElement;
  assert((bitsPerElement == 32) || (bitsPerElement == 16) || (bitsPerElement == 8));

  if (channels == 1) {
    void* dataSource;
    if (bitsPerElement == 32) {
      dataSource = hostBuffer->_data;
    } else if ((bitsPerElement == 16) || (bitsPerElement == 8)) {
      dataSource = hostBuffer->_quantizedData;
    } else {
      assert(false); // Should never get here
      dataSource = NULL;
    }
    glTexImage2D(
      GL_TEXTURE_2D,
      0,
      GL_RGBA,
      width,
      height,
      0,
      GL_RGBA,
      GL_UNSIGNED_BYTE,
      dataSource);
  } else if (channels == 3) {
    assert(bitsPerElement == 32);
#ifdef GL_RGB32F_ARB
    glTexImage2D(
      GL_TEXTURE_2D,
      0,
      GL_RGB32F_ARB,
      width,
      height,
      0,
      GL_RGB,
      GL_FLOAT,
      hostBuffer->_data);
#else
    assert(false); // Float not supported
#endif
  } else if (channels == 4) {
    assert(bitsPerElement == 32);
#ifdef GL_RGBA32F_ARB
    glTexImage2D(
      GL_TEXTURE_2D,
      0,
      GL_RGBA32F_ARB,
      width,
      height,
      0,
      GL_RGBA,
      GL_FLOAT,
      hostBuffer->_data);
#else
    assert(false); // Float not supported
#endif
  } else {
    assert(false); // Bad number of channels
  }
  CHECK_GL_ERROR();
}

#endif // USE_OPENGL