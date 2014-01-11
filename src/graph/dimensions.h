//
//  dimensions.h
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#ifndef INCLUDE_DIMENSIONS_H
#define INCLUDE_DIMENSIONS_H

#include "stdlib.h"
#include "stdio.h"

#define DIMENSIONS_MAX_LENGTH (5)

class Dimensions
{
public:
  Dimensions(int a);
  Dimensions(int a, int b);
  Dimensions(int a, int b, int c);
  Dimensions(int a, int b, int c, int d);
  Dimensions(int a, int b, int c, int d, int e);
  Dimensions(int* dims, int length);

  int elementCount();
  int byteCount();
  int last();

  char* debugString();

  int _length;
  int _dims[DIMENSIONS_MAX_LENGTH];
};

inline Dimensions::Dimensions(int a)
{
  _dims[0] = a;
  _length = 1;
}

inline Dimensions::Dimensions(int a, int b)
{
  _dims[0] = a;
  _dims[1] = b;
  _length = 2;
}

inline Dimensions::Dimensions(int a, int b, int c)
{
  _dims[0] = a;
  _dims[1] = b;
  _dims[2] = c;
  _length = 3;
}

inline Dimensions::Dimensions(int a, int b, int c, int d)
{
  _dims[0] = a;
  _dims[1] = b;
  _dims[2] = c;
  _dims[3] = d;
  _length = 4;
}

inline Dimensions::Dimensions(int a, int b, int c, int d, int e)
{
  _dims[0] = a;
  _dims[1] = b;
  _dims[2] = c;
  _dims[3] = d;
  _dims[4] = e;
  _length = 5;
}

inline Dimensions::Dimensions(int* dims, int length)
{
  if (length > DIMENSIONS_MAX_LENGTH) {
    fprintf(stderr, "Length too large in Dimensions::Dimensions(): %d, max is %d", length, DIMENSIONS_MAX_LENGTH);
  }
  for (int i = 0; i < MIN(length, DIMENSIONS_MAX_LENGTH); i += 1) {
    _dims[i] = dims[i];
  }
  _length = length;
}

inline int Dimensions::last()
{
  return _dims[_length - 1];
}

inline int Dimensions::elementCount()
{
  int result = 1;
  for (int index = 0; index < _length; index += 1)
  {
    result *= _dims[index];
  }
  return result;
}

inline int Dimensions::byteCount()
{
  return elementCount() * sizeof(jpfloat_t);
}

inline char* Dimensions::debugString()
{
  // Will be leaked, but don't care during debugging
  char* result = (char*)(malloc(MAX_DEBUG_STRING_LEN));
  switch (_length)
  {
    case 1:
      snprintf(result, MAX_DEBUG_STRING_LEN, "(%d)",
        _dims[0]);
    break;
    case 2:
      snprintf(result, MAX_DEBUG_STRING_LEN, "(%d, %d)",
        _dims[0], _dims[1]);
    break;
    case 3:
      snprintf(result, MAX_DEBUG_STRING_LEN, "(%d, %d, %d)",
        _dims[0], _dims[1],  _dims[2]);
    break;
    case 4:
      snprintf(result, MAX_DEBUG_STRING_LEN, "(%d, %d, %d, %d)",
        _dims[0], _dims[1],  _dims[2], _dims[3]);
    break;
    case 5:
      snprintf(result, MAX_DEBUG_STRING_LEN, "(%d, %d, %d, %d, %d)",
        _dims[0], _dims[1],  _dims[2], _dims[3], _dims[4]);
    break;
    default:
      snprintf(result, MAX_DEBUG_STRING_LEN, "Bad length in Dimensions::debugString(): %d", _length);
    break;
  }
  return result;
}

#endif // INCLUDE_DIMENSIONS_H
