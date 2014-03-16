//
//  glcontext.cpp
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#ifdef USE_OPENGL

#include "glcontext.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "glprogram.h"
#include "glbuffer.h"
#include "buffer.h"

const int maxInputBuffers = 8;

static int g_nextWindowIndex = 0;

GLContext::GLContext() {
  _inputs = (GLBuffer**)(malloc(sizeof(GLBuffer*) * maxInputBuffers));
  for (int index = 0; index < maxInputBuffers; index += 1) {
    _inputs[index] = NULL;
  }
  _output = NULL;

  int glut_argv = 1;
  char* glut_argc[] = { "foo" };
  glutInit(&glut_argv, glut_argc);

  const int windowNameLength = 256;
  char windowName[windowNameLength];
  snprintf(windowName, windowNameLength, "JPCNN Window #%d", g_nextWindowIndex);
  g_nextWindowIndex += 1;

  _glutWindowHandle = glutCreateWindow(windowName);
  CHECK_GL_ERROR();

  glGenFramebuffers(1, &_framebuffer);
  CHECK_GL_ERROR();
  glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
  CHECK_GL_ERROR();
}

GLContext::~GLContext() {
  glDeleteFramebuffers(1, &_framebuffer);
  glutDestroyWindow(_glutWindowHandle);
}

void GLContext::setProgram(GLProgram* program) {
  _program = program;
}

void GLContext::setOutputBuffer(GLBuffer* buffer) {
  _output = buffer;

  const Dimensions& outputDims = _output->_hostBuffer->_dims;
  const int width = outputDims[1];
  const int height = outputDims[0];

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, width, 0.0, height, -1.0f, 1.0f);

  glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
  CHECK_GL_ERROR();

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _output->_textureId, 0);
  GLint framebufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (framebufferStatus != GL_FRAMEBUFFER_COMPLETE) {
    fprintf(stderr, "glCheckFramebufferStatus() failed with 0x%x\n", framebufferStatus);
  }
  CHECK_GL_ERROR();
}

void GLContext::runProgram() {

  const Dimensions outputDims = _output->_hostBuffer->_dims;
  const jpfloat_t outputWidth = outputDims[1];
  const jpfloat_t outputHeight = outputDims[0];

  const jpfloat_t viewBottom = 0;
  const jpfloat_t viewTop = outputHeight;
  const jpfloat_t viewLeft = 0;
  const jpfloat_t viewRight = outputWidth;

  GLfloat vertices[] = {
    viewLeft, viewBottom,
    viewLeft, viewTop,
    viewRight, viewBottom,
    viewRight, viewTop,
  };
  const int elementsPerVertex = 2;
  const int vertexCount = 4;
  const size_t singleVertexByteCount = (elementsPerVertex * sizeof(GLfloat));
  const size_t allVerticesByteCount = (vertexCount * singleVertexByteCount);

  GLuint programId = _program->_programId;
  glUseProgram(programId);
  CHECK_GL_ERROR();

  GLuint vertexBufferId;
  glGenBuffers(1, &vertexBufferId);
  glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
  CHECK_GL_ERROR();
  glBufferData(GL_ARRAY_BUFFER, allVerticesByteCount, vertices, GL_STATIC_DRAW);
  CHECK_GL_ERROR();
  GLuint positionAttrib = glGetAttribLocation(programId, "aVertexPosition");
  glVertexAttribPointer(positionAttrib, elementsPerVertex, GL_FLOAT, GL_FALSE, singleVertexByteCount, 0);
  CHECK_GL_ERROR();
  glEnableVertexAttribArray(positionAttrib);
  CHECK_GL_ERROR();

  GLfloat texCoords[] = {
    0, 0,
    0, outputHeight,
    outputWidth, 0,
    outputWidth, outputHeight,
  };
  const int elementsPerTexCoord = 2;
  const int texCoordCount = 4;
  const size_t singleTexCoordByteCount = (elementsPerTexCoord * sizeof(GLfloat));
  const size_t allTexCoordsByteCount = (texCoordCount * singleTexCoordByteCount);
  GLuint texAttrib = glGetAttribLocation(programId, "aTexCoord");
  if (texAttrib == 0xffffffff) {
    fprintf(stderr, "Couldn't find attribute 'aTexCoord' in program\n");
    return;
  }
  GLuint texCoordsBufferId;
  glGenBuffers(1, &texCoordsBufferId);
  glBindBuffer(GL_ARRAY_BUFFER, texCoordsBufferId);
  CHECK_GL_ERROR();
  glBufferData(GL_ARRAY_BUFFER, allTexCoordsByteCount, texCoords, GL_STATIC_DRAW);
  CHECK_GL_ERROR();
  glVertexAttribPointer(texAttrib, elementsPerTexCoord, GL_FLOAT, GL_FALSE, singleVertexByteCount, 0);
  CHECK_GL_ERROR();
  glEnableVertexAttribArray(texAttrib);
  CHECK_GL_ERROR();

  _program->bindInputBuffers();

  glDrawArrays(GL_TRIANGLE_STRIP, 0, vertexCount);
  CHECK_GL_ERROR();
}

void GLContext::copyOutputToHost(Buffer* hostBuffer) {
  const Dimensions& outputDims = hostBuffer->_dims;
  assert(hostBuffer->_dims == outputDims);
  const int width = outputDims[1];
  const int height = outputDims[0];
  const int channels = outputDims[2];
//  jpfloat_t* outputData = hostBuffer->_data;
  jpfloat_t* outputData = (jpfloat_t*)(malloc(width * height * channels * sizeof(jpfloat_t)));
  GLint channelFormat;
  if (channels == 1) {
    channelFormat = GL_RED;
  } else if (channels == 3) {
    channelFormat = GL_RGB;
  } else if (channels == 4) {
    channelFormat = GL_RGBA;
  } else {
    assert(false); // Bad number of channels
  }
  glReadPixels(0, 0, width, height, channelFormat, GL_FLOAT, outputData);
  memcpy(hostBuffer->_data, outputData, (width * height * channels * sizeof(jpfloat_t)));
  CHECK_GL_ERROR();
}

#endif // USE_OPENGL