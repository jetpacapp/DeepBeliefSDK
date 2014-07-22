//
//  glheaders.h
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#ifndef INCLUDE_GLHEADERS_H
#define INCLUDE_GLHEADERS_H

#include "jpcnn.h"

#if __APPLE__
  #include "TargetConditionals.h"
  #if TARGET_OS_IPHONE
    #define OPENGL_IPHONE
    #include <OpenGLES/ES2/gl.h>
    #include <OpenGLES/ES2/glext.h>
  #elif TARGET_OS_MAC
    #include <OpenGL/gl.h>
    #include <OpenGL/glext.h>
  #endif
#elif defined(TARGET_PI)
  #include <EGL/egl.h>
  #include <EGL/eglext.h>
  #include <GLES2/gl2.h>
  #include <bcm_host.h>
#else
  #include <gl.h>
  #include <glext.h>
#endif

#endif // INCLUDE_GLHEADERS_H