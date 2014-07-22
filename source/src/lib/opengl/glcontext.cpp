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

GLContext::GLContext() {
  _inputs = (GLBuffer**)(malloc(sizeof(GLBuffer*) * maxInputBuffers));
  for (int index = 0; index < maxInputBuffers; index += 1) {
    _inputs[index] = NULL;
  }
  _output = NULL;

  createContextHandle();
  CHECK_GL_ERROR();

  glGenFramebuffers(1, &_framebuffer);
  CHECK_GL_ERROR();
  glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
  CHECK_GL_ERROR();
}

GLContext::~GLContext() {
  glDeleteFramebuffers(1, &_framebuffer);
  destroyContextHandle();
}

void GLContext::setProgram(GLProgram* program) {
  _program = program;
}

void GLContext::setOutputBuffer(GLBuffer* buffer) {
  _output = buffer;

  const Dimensions& outputDims = _output->_hostBuffer->_dims;
  const int width = outputDims[1];
  const int height = outputDims[0];

	glViewport(0, 0, width, height);

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

  const GLfloat a = (2.0f / outputWidth);
  const GLfloat b = (2.0f / outputHeight);
  const GLfloat c = -1.0f;

  const GLfloat tx = -1.0f;
  const GLfloat ty = -1.0f;
  const GLfloat tz = 0.0f;

  GLfloat orthoValues[16] = {
      a, 0, 0, 0,
      0, b, 0, 0,
      0, 0, c, 0,
      tx, ty, tz, 1
  };
  _program->setUniformMatrix4fv("modelViewProjectionMatrix", orthoValues);

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
  jpfloat_t* outputData = hostBuffer->_data;
  GLint dataFormat;
  GLint channelFormat;
  if (channels == 1) {
    dataFormat = GL_UNSIGNED_BYTE;
    channelFormat = GL_RGBA;
  } else if (channels == 3) {
    dataFormat = GL_FLOAT;
    channelFormat = GL_RGB;
  } else if (channels == 4) {
    dataFormat = GL_FLOAT;
    channelFormat = GL_RGBA;
  } else {
    assert(false); // Bad number of channels
  }
  glReadPixels(0, 0, width, height, channelFormat, dataFormat, outputData);
  CHECK_GL_ERROR();
}

// OpenGL context support for non-iOS platforms
#if defined(TARGET_PI)

typedef struct SGLSurfaceInfoStruct {
  EGL_DISPMANX_WINDOW_T window;
  EGLDisplay display;
  EGLSurface surface;
  EGLContext context;
} SGLSurfaceInfo;

void GLContext::createContextHandle() {
  SGLSurfaceInfo* surface = (SGLSurfaceInfo*)(malloc(sizeof(SGLSurfaceInfo)));
	surface->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if (surface->display == EGL_NO_DISPLAY) {
    fprintf(stderr, "GLContext::createContextHandle(): Failed to get display.\n");
    return;
	}
	int majorVersion;
  int minorVersion;
	EGLBoolean initializeResult = eglInitialize(surface->display, &majorVersion, &minorVersion);
	if (initializeResult == EGL_FALSE) {
    fprintf(stderr, "GLContext::createContextHandle(): Failed to initialize.\n");
    return;
	}

	EGLBoolean bindResult = eglBindAPI(EGL_OPENGL_ES_API);
	if(bindResult ==EGL_FALSE) {
    fprintf(stderr, "GLContext::createContextHandle(): Failed to bind.\n");
    return;
	}

  const EGLint configAttributes[] = {
    EGL_RED_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_BLUE_SIZE, 8,
    EGL_ALPHA_SIZE, 8,
    EGL_DEPTH_SIZE, 16,
    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES_BIT,
    EGL_NONE
  };
  EGLConfig config;
  EGLint configCount;
  eglChooseConfig(surface->display, configAttributes, &config, 1, &configCount);

	const EGLint contextAttributes[] = {
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE
	};
	surface->context = eglCreateContext(surface->display, config, EGL_NO_CONTEXT, contextAttributes);
	if(surface->context == EGL_NO_CONTEXT) {
    fprintf(stderr, "GLContext::createContextHandle(): Failed to create context.\n");
    return;
	}

  const int maxWidth = 1920;
  const int maxHeight = 1080;

	VC_RECT_T dstRect;
	dstRect.x = 0;
	dstRect.y = 0;
  dstRect.width = maxWidth;
  dstRect.height = maxHeight;

	VC_RECT_T srcRect;
	srcRect.x = 0;
	srcRect.y = 0;
	srcRect.width = maxWidth << 16;
	srcRect.height = maxHeight << 16;

  DISPMANX_DISPLAY_HANDLE_T dispmanDisplay = vc_dispmanx_display_open(0);
	DISPMANX_UPDATE_HANDLE_T dispmanUpdate = vc_dispmanx_update_start(0);
	DISPMANX_ELEMENT_HANDLE_T dispmanElement = vc_dispmanx_element_add(
    dispmanUpdate,
    dispmanDisplay,
		0,
    &dstRect,
    0,
    &srcRect,
    DISPMANX_PROTECTION_NONE,
    0,
    0,
    DISPMANX_NO_ROTATE);
	vc_dispmanx_update_submit_sync(dispmanUpdate);

	surface->window.element = dispmanElement;
	surface->window.width = maxWidth;
	surface->window.height = maxHeight;

	surface->surface = eglCreateWindowSurface(surface->display, config, &surface->window, NULL );
	if (surface->surface == EGL_NO_SURFACE) {
    fprintf(stderr, "GLContext::createContextHandle(): Failed to create window surface.\n");
    return;
  }
	EGLBoolean makeCurrentResult = eglMakeCurrent(surface->display, surface->surface, surface->surface, surface->context);
  if (makeCurrentResult == EGL_FALSE) {
    fprintf(stderr, "GLContext::createContextHandle(): Failed to switch to the context.\n");
    return;
  }
  _contextHandle.pointer = (void*)(surface);
}

void GLContext::destroyContextHandle() {
  SGLSurfaceInfo* surface = (SGLSurfaceInfo*)(_contextHandle.pointer);
  eglSwapBuffers(surface->display, surface->surface);
  eglMakeCurrent(surface->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
  eglDestroySurface(surface->display, surface->surface);
  eglDestroyContext(surface->display, surface->context);
  eglTerminate(surface->display);
  free(surface);
}

#elif !__APPLE__ || !TARGET_OS_IPHONE

void GLContext::createContextHandle() {
  static int nextWindowIndex = 0;
  int glut_argv = 1;
  char* glut_argc[] = { "foo" };
  glutInit(&glut_argv, glut_argc);

  const int windowNameLength = 256;
  char windowName[windowNameLength];
  snprintf(windowName, windowNameLength, "JPCNN Window #%d", nextWindowIndex);
  nextWindowIndex += 1;

  const int windowHandle = glutCreateWindow(windowName);

  _contextHandle.integer = windowHandle;
}

void GLContext::destroyContextHandle() {
  const int windowHandle = _contextHandle.integer;
  glutDestroyWindow(windowHandle);
}

#endif

#endif // USE_OPENGL