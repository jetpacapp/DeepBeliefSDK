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

class Dimensions {
public:
  Dimensions(int a);
  Dimensions(int a, int b);
  Dimensions(int a, int b, int c);
  Dimensions(int a, int b, int c, int d);
  Dimensions(int a, int b, int c, int d, int e);
  Dimensions(const int* dims, int length);
  Dimensions(const Dimensions & other);

  int elementCount() const;
  int byteCount() const;
  int last() const;
  int operator[](const int index) const;
  int offset(int aIndex) const;
  int offset(int aIndex, int bIndex) const;
  int offset(int aIndex, int bIndex, int cIndex) const;
  int offset(int aIndex, int bIndex, int cIndex, int dIndex) const;
  int offset(int aIndex, int bIndex, int cIndex, int dIndex, int eIndex) const;
  Dimensions removeDimensions(int howMany) const;
  bool operator==(const Dimensions & other) const;

  char* debugString() const;

  int _length;
  int _dims[DIMENSIONS_MAX_LENGTH];
};

inline Dimensions::Dimensions(int a) {
  _dims[0] = a;
  _length = 1;
}

inline Dimensions::Dimensions(int a, int b) {
  _dims[0] = a;
  _dims[1] = b;
  _length = 2;
}

inline Dimensions::Dimensions(int a, int b, int c) {
  _dims[0] = a;
  _dims[1] = b;
  _dims[2] = c;
  _length = 3;
}

inline Dimensions::Dimensions(int a, int b, int c, int d) {
  _dims[0] = a;
  _dims[1] = b;
  _dims[2] = c;
  _dims[3] = d;
  _length = 4;
}

inline Dimensions::Dimensions(int a, int b, int c, int d, int e) {
  _dims[0] = a;
  _dims[1] = b;
  _dims[2] = c;
  _dims[3] = d;
  _dims[4] = e;
  _length = 5;
}

inline Dimensions::Dimensions(const int* dims, int length) {
  if (length > DIMENSIONS_MAX_LENGTH) {
    fprintf(stderr, "Length too large in Dimensions::Dimensions(): %d, max is %d", length, DIMENSIONS_MAX_LENGTH);
  }
  for (int i = 0; i < MIN(length, DIMENSIONS_MAX_LENGTH); i += 1) {
    _dims[i] = dims[i];
  }
  _length = length;
}

inline Dimensions::Dimensions(const Dimensions & other) {
  for (int i = 0; i < other._length; i += 1) {
    _dims[i] = other._dims[i];
  }
  _length = other._length;
}

inline int Dimensions::last() const {
  return _dims[_length - 1];
}

inline int Dimensions::elementCount() const {
  int result = 1;
  for (int index = 0; index < _length; index += 1) {
    result *= _dims[index];
  }
  return result;
}

inline int Dimensions::byteCount() const {
  return elementCount() * sizeof(jpfloat_t);
}

inline int Dimensions::operator[](const int index) const {
  if (index >= _length) {
    return -1;
  }
  return _dims[index];
}

inline int Dimensions::offset(int aIndex) const {
  const int size0 = 1;
  return (aIndex * size0);
}

inline int Dimensions::offset(int aIndex, int bIndex) const {
  const int size0 = 1;
  const int size1 = (_dims[1] * size0);
  return ((aIndex * size1) + (bIndex * size0));
}

inline int Dimensions::offset(int aIndex, int bIndex, int cIndex) const {
  const int size0 = 1;
  const int size1 = (_dims[2] * size0);
  const int size2 = (_dims[1] * size1);
  return ((aIndex * size2) + (bIndex * size1) + (cIndex * size0));
}

inline int Dimensions::offset(int aIndex, int bIndex, int cIndex, int dIndex) const {
  const int size0 = 1;
  const int size1 = (_dims[3] * size0);
  const int size2 = (_dims[2] * size1);
  const int size3 = (_dims[1] * size2);
  return ((aIndex * size3) + (bIndex * size2) + (cIndex * size1) + (dIndex * size0));
}

inline int Dimensions::offset(int aIndex, int bIndex, int cIndex, int dIndex, int eIndex) const {
  const int size0 = 1;
  const int size1 = (_dims[4] * size0);
  const int size2 = (_dims[3] * size1);
  const int size3 = (_dims[2] * size2);
  const int size4 = (_dims[1] * size3);
  return ((aIndex * size4) + (bIndex * size3) + (cIndex * size2) + (dIndex * size1) + (eIndex * size0));
}

inline char* Dimensions::debugString() const {
  // Will be leaked, but don't care during debugging
  char* result = (char*)(malloc(MAX_DEBUG_STRING_LEN));
  switch (_length) {
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

inline Dimensions Dimensions::removeDimensions(int howMany) const {
  if ((_length - howMany) <= 0) {
    return Dimensions(0);
  }
  int newDims[DIMENSIONS_MAX_LENGTH];
  for (int i = howMany; i < _length; i += 1) {
    newDims[i - howMany] = _dims[i];
  }
  return Dimensions(newDims, (_length - howMany));
}

inline bool Dimensions::operator==(const Dimensions & other) const {
  if (_length != other._length) {
    return false;
  }
  for (int index = 0; index < _length; index += 1) {
    if (_dims[index] != other._dims[index]) {
      return false;
    }
  }
  return true;
}

#endif // INCLUDE_DIMENSIONS_H
