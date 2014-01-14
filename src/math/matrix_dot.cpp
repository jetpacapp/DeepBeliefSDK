//
//  matrix_dot.cpp
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#include "matrix_ops.h"

#include <assert.h>

#include "buffer.h"

Buffer* matrix_dot(Buffer* a, Buffer* b) {
  const Dimensions aDims = a->_dims;
  const Dimensions bDims = b->_dims;
  assert(bDims._length == 1);
  const int dotWidth = bDims[0];
  assert(dotWidth == aDims[aDims._length-1]);

  Dimensions outputDims(aDims._dims, aDims._length);
  outputDims._dims[outputDims._length - 1] = bDims[bDims._length - 1];
  Buffer* output = new Buffer(outputDims);

  return output;
}
