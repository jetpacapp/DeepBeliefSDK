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
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>

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

  if (channels == 1) {
    glTexImage2D(
      GL_TEXTURE_2D,
      0,
      GL_RGBA,
      width,
      height,
      0,
      GL_RGBA,
      GL_UNSIGNED_BYTE,
      hostBuffer->_data);
  } else if (channels == 3) {
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
  } else if (channels == 4) {
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
  } else {
    assert(false); // Bad number of channels
  }
  CHECK_GL_ERROR();
}

#endif // USE_OPENGL