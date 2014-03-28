//
//  glprogram.h
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#ifndef INCLUDE_GLPROGRAM_H
#define INCLUDE_GLPROGRAM_H

#include "glheaders.h"

class GLContext;
class GLBuffer;

class GLProgram {
public:
  GLProgram(GLContext* context, const char* vertexShaderText, const char* fragmentShaderText, const char** uniformNames, GLint* uniformIds, int howManyUniforms);
  virtual ~GLProgram();

  GLint getUniformIdFromName(const char* name);
  void setUniform1i(const char* name, int value);
  void setUniform1f(const char* name, float value);
  void setUniform2f(const char* name, float x, float y);
  void setUniform3f(const char* name, float x, float y, float z);
  void setUniform4f(const char* name, float x, float y, float z, float w);
  void setUniformMatrix4fv(const char* name, float* values);
  void clearInputBuffers();
  void setInputBuffer(const char* name, GLBuffer* buffer);
  void bindInputBuffers();

  char** _uniformNames;
  GLint* _uniformIds;
  int _howManyUniforms;

  GLBuffer** _inputBuffers;
  char** _inputBufferNames;
  int _howManyInputBuffers;

  GLContext* _context;
  GLint _programId;
};

#endif // INCLUDE_GLPROGRAM_H
