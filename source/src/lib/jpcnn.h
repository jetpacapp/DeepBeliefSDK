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

// Define this if you want details of the operations being
// being performed printed to stderr
//#define DO_LOG_OPERATIONS

#define USE_GEMM
//#define USE_NAIVE

#endif // INCLUDE_JPCNN_H
