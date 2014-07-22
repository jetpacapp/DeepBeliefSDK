//
//  glcontext.h
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#ifndef INCLUDE_GLCONTEXT_H
#define INCLUDE_GLCONTEXT_H

#include "glheaders.h"

#if (__APPLE__ && TARGET_OS_IPHONE) || defined(TARGET_PI)
#define gluErrorString(x) ("")
#else
#include <GLUT/glut.h>
#endif

#if defined(DEBUG)
#define CHECK_GL_ERROR() do {                                 \
  /*glFinish();*/                                             \
  GLint error = glGetError();                                 \
  if (error != GL_NO_ERROR) {                                 \
    fprintf(stderr, "OpenGL Error %s 0x%x at %s:%d (%s)\n",   \
      gluErrorString(error), error,                           \
      __FILE__, __LINE__,  __PRETTY_FUNCTION__);              \
    assert(false);                                            \
  }} while(0)
#else // DEBUG
#define CHECK_GL_ERROR() (void)(0)
#endif // DEBUG

class GLProgram;
class GLBuffer;
class Buffer;

class GLContext {
public:
  GLContext();
  virtual ~GLContext();

  void setProgram(GLProgram* program);
  void setInputBuffer(GLBuffer* buffer, int index);
  void setOutputBuffer(GLBuffer* buffer);
  void runProgram();
  void copyOutputToHost(Buffer* hostBuffer);
  void createContextHandle();
  void destroyContextHandle();

  GLProgram* _program;
  GLBuffer** _inputs;
  GLBuffer* _output;

  union {
    void* pointer;
    int integer;
  } _contextHandle;
  GLuint _framebuffer;
};

#endif // INCLUDE_GLCONTEXT_H
