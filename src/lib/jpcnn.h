//
//  jpcnn.h
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#ifndef INCLUDE_JPCNN_H
#define INCLUDE_JPCNN_H

#define MAX_DEBUG_STRING_LEN (1024)

#ifndef MAX
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#endif // MAX

#ifndef MIN
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#endif // MIN

typedef float jpfloat_t;

// Whether to use the cuda-convnet constants for buffer sizes, etc
//#define USE_CUDACONVNET_DEFS

#endif // INCLUDE_JPCNN_H
