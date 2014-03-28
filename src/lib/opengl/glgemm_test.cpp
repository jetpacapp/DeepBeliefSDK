//
//  glgemm_test.cpp
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#ifdef USE_OPENGL

#include <assert.h>

#include "matrix_ops.h"
#include "buffer.h"
#include "glgemm.h"

int main(int argc, char** argv) {

  test_gl_gemm();

  return 0;
}

#endif // USE_OPENGL