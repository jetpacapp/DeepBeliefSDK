//
//  glcontext.mm
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#include "glcontext.h"

// OpenGL context support for iOS platforms
#if __APPLE__ && TARGET_OS_IPHONE

#include <OpenGLES/EAGL.h>

void GLContext::createContextHandle() {
  EAGLContext* context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
  [EAGLContext setCurrentContext: context];
  _contextHandle.pointer = (void*)(context);
}

void GLContext::destroyContextHandle() {
  EAGLContext* context = (EAGLContext*)(_contextHandle.pointer);
  [context dealloc];
}

#endif