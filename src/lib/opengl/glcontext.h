//
//  glcontext.h
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#ifndef INCLUDE_GLCONTEXT_H
#define INCLUDE_GLCONTEXT_H

#include <OpenGL/gl.h>
#include <GLUT/glut.h>

#define CHECK_GL_ERROR() do {                                 \
  GLint error = glGetError();                                 \
  if (error != GL_NO_ERROR) {                                 \
    fprintf(stderr, "OpenGL Error %s 0x%x at %s:%d (%s)\n",   \
      gluErrorString(error), error,                           \
      __FILE__, __LINE__,  __PRETTY_FUNCTION__);              \
    assert(false);                                            \
  }} while(0)

class GLProgram;
class GLBuffer;

class GLContext {
public:
  GLContext();
  virtual ~GLContext();

  void setProgram(GLProgram* program);
  void setInputBuffer(GLBuffer* buffer, int index);
  void setOutputBuffer(GLBuffer* buffer);
  void runProgram();
  void copyOutputToHost();

  GLProgram* _program;
  GLBuffer** _inputs;
  GLBuffer* _output;

  int _glutWindowHandle;
  GLuint _framebuffer;
};

#endif // INCLUDE_GLCONTEXT_H
