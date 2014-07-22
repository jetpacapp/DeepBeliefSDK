//
//  glprogram.cpp
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#ifdef USE_OPENGL

#include "glprogram.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "glheaders.h"
#include "cstring_helpers.h"
#include "glcontext.h"
#include "glbuffer.h"

static GLint load_glsl_program(const char* vertexShaderText, const char* fragmentShaderText, char** uniformNames, GLint* uniformIds, int howManyUniforms);
static int compile_shader(const char* shaderText, GLenum type, GLint* outShaderId);
static int link_program(GLint programId);

const int maxInputBuffers = 8;

GLProgram::GLProgram(GLContext* context, const char* vertexShaderText, const char* fragmentShaderText, const char** uniformNames, GLint* uniformIds, int howManyUniforms) {
  _context = context;

  _howManyUniforms = howManyUniforms;
  _uniformNames = (char**)(malloc(_howManyUniforms * sizeof(char*)));
  _uniformIds = (GLint*)(malloc(_howManyUniforms * sizeof(GLint)));
  for (int index = 0; index < _howManyUniforms; index += 1) {
    _uniformNames[index] = malloc_and_copy_string(uniformNames[index]);
  }

  _inputBuffers = (GLBuffer**)(malloc(maxInputBuffers * sizeof(GLBuffer*)));
  _inputBufferNames = (char**)(malloc(maxInputBuffers * sizeof(char*)));
  for (int index = 0; index < maxInputBuffers; index += 1) {
    _inputBuffers[index] = NULL;
    _inputBufferNames[index] = NULL;
  }
  _howManyInputBuffers = 0;

#ifdef OPENGL_IPHONE
  const char* prefixLine = "precision highp float;\n";
  const size_t prefixedLength = (strlen(prefixLine) + strlen(fragmentShaderText) + 1);
  char* prefixedFragmentShaderText = (char*)(malloc(prefixedLength));
  snprintf(prefixedFragmentShaderText, prefixedLength, "%s%s", prefixLine, fragmentShaderText);
  _programId = load_glsl_program(
    vertexShaderText,
    prefixedFragmentShaderText,
    _uniformNames,
    _uniformIds,
    _howManyUniforms);
  free(prefixedFragmentShaderText);
#else // OPENGL_IPHONE
  _programId = load_glsl_program(
    vertexShaderText,
    fragmentShaderText,
    _uniformNames,
    _uniformIds,
    _howManyUniforms);
#endif // OPENGL_IPHONE
}

GLProgram::~GLProgram() {
  glDeleteProgram(_programId);
  for (int index = 0; index < _howManyUniforms; index += 1) {
    free(_uniformNames[index]);
  }
  free(_uniformNames);
  free(_uniformIds);
  for (int index = 0; index < _howManyInputBuffers; index += 1) {
    // We don't own the input buffers, so just free the array holding
    // the pointers to them, but do free the name strings.
    free(_inputBufferNames[index]);
  }
  free(_inputBuffers);
}

GLint GLProgram::getUniformIdFromName(const char* name) {
  GLint foundId = -1;
  for (int index = 0; index < _howManyUniforms; index += 1) {
    if (strcmp(name, _uniformNames[index]) == 0) {
      foundId = _uniformIds[index];
    }
  }
  if (foundId == -1) {
    fprintf(stderr, "getUniformIdFromName('%s') not found\n", name);
  }
  return foundId;
}

void GLProgram::setUniform1i(const char* name, int value) {
  glUseProgram(_programId);
  GLint id = getUniformIdFromName(name);
  assert(id != -1);
  glUniform1i(id, value);
  CHECK_GL_ERROR();
}

void GLProgram::setUniform1f(const char* name, float value) {
  glUseProgram(_programId);
  GLint id = getUniformIdFromName(name);
  assert(id != -1);
  glUniform1f(id, value);
  CHECK_GL_ERROR();
}

void GLProgram::setUniform2f(const char* name, float x, float y) {
  glUseProgram(_programId);
  GLint id = getUniformIdFromName(name);
  assert(id != -1);
  glUniform2f(id, x, y);
  CHECK_GL_ERROR();
}

void GLProgram::setUniform3f(const char* name, float x, float y, float z) {
  glUseProgram(_programId);
  GLint id = getUniformIdFromName(name);
  assert(id != -1);
  glUniform3f(id, x, y, z);
  CHECK_GL_ERROR();
}

void GLProgram::setUniform4f(const char* name, float x, float y, float z, float w) {
  glUseProgram(_programId);
  GLint id = getUniformIdFromName(name);
  assert(id != -1);
  glUniform4f(id, x, y, z, w);
  CHECK_GL_ERROR();
}

void GLProgram::setUniformMatrix4fv(const char* name, float* values) {
  glUseProgram(_programId);
  GLint id = getUniformIdFromName(name);
  assert(id != -1);
  glUniformMatrix4fv(id, 1, GL_FALSE, values);
}

void GLProgram::clearInputBuffers() {
  _howManyInputBuffers = 0;
}

void GLProgram::setInputBuffer(const char* name, GLBuffer* buffer) {
  assert(_howManyInputBuffers < maxInputBuffers);
  _inputBuffers[_howManyInputBuffers] = buffer;
  _inputBufferNames[_howManyInputBuffers] = malloc_and_copy_string(name);
  _howManyInputBuffers += 1;
}

void GLProgram::bindInputBuffers() {
  for (int index = 0; index < _howManyInputBuffers; index += 1) {
    GLBuffer* buffer = _inputBuffers[index];
    glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(GL_TEXTURE_2D, buffer->_textureId);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    char* name = _inputBufferNames[index];
    GLint uniformId = getUniformIdFromName(name);
    assert(uniformId != -1);
    glUniform1i(uniformId, index);
    CHECK_GL_ERROR();
  }
}

GLint load_glsl_program(const char* vertexShaderText, const char* fragmentShaderText, char** uniformNames, GLint* uniformIds, int howManyUniforms) {

  GLint vertexShaderId;
  if (!compile_shader(vertexShaderText, GL_VERTEX_SHADER, &vertexShaderId)) {
    fprintf(stderr, "Failed to compile vertex shader\n");
    return 0;
  }

  GLint fragmentShaderId;
  if (!compile_shader(fragmentShaderText, GL_FRAGMENT_SHADER, &fragmentShaderId)) {
    fprintf(stderr, "Failed to compile fragment shader\n");
    return 0;
  }

  GLint programId = glCreateProgram();
  glAttachShader(programId, vertexShaderId);
  glAttachShader(programId, fragmentShaderId);

  // Link program.
  if (!link_program(programId)) {
    fprintf(stderr, "Failed to link program: %d", programId);
    return 0;
  }

  // Get uniform locations.
  for (int index = 0; index < howManyUniforms; index += 1) {
    const char* uniformName = uniformNames[index];
    uniformIds[index] = glGetUniformLocation(programId, uniformName);
  }

  // Release vertex and fragment shaders.
  glDetachShader(programId, vertexShaderId);
  glDeleteShader(vertexShaderId);
  glDetachShader(programId, fragmentShaderId);
  glDeleteShader(fragmentShaderId);

  CHECK_GL_ERROR();

  return programId;
}

int compile_shader(const char* shaderText, GLenum type, GLint* outShaderId) {

  GLint shaderId = glCreateShader(type);
  glShaderSource(shaderId, 1, &shaderText, NULL);
  glCompileShader(shaderId);

#if defined(DEBUG)
  GLint logLength;
  glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &logLength);
  if (logLength > 0) {
    GLchar *log = (GLchar *)malloc(logLength);
    glGetShaderInfoLog(shaderId, logLength, &logLength, log);
    fprintf(stderr, "Shader compile log:\n%s", log);
    free(log);
  }
#endif

  GLint status;
  glGetShaderiv(shaderId, GL_COMPILE_STATUS, &status);
  if (status == 0) {
    glDeleteShader(shaderId);
    return 0;
  }

  *outShaderId = shaderId;
  return 1;
}

int link_program(GLint programId) {
  glLinkProgram(programId);

#if defined(DEBUG)
  GLint logLength;
  glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &logLength);
  if (logLength > 0) {
    GLchar *log = (GLchar *)malloc(logLength);
    glGetProgramInfoLog(programId, logLength, &logLength, log);
    fprintf(stderr, "Program link log:\n%s\n", log);
    free(log);
  }
#endif

  GLint status;
  glGetProgramiv(programId, GL_LINK_STATUS, &status);
  if (status == 0) {
    return 0;
  }

  return 1;
}

#endif // USE_OPENGL

