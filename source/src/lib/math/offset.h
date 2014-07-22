//
//  offset.h
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#ifndef INCLUDE_OFFSET_H
#define INCLUDE_OFFSET_H

#include "stdlib.h"
#include "stdio.h"

#define OFFSET_MAX_LENGTH (5)

class Offset {
public:
  Offset(int a);
  Offset(int a, int b);
  Offset(int a, int b, int c);
  Offset(int a, int b, int c, int d);
  Offset(int a, int b, int c, int d, int e);
  Offset(const int* offset, int length);
  Offset(const Offset & other);

  int operator[](const int index) const;
  bool operator==(const Offset& other) const;

  char* debugString() const;

  int _length;
  int _offset[OFFSET_MAX_LENGTH];
};

inline Offset::Offset(int a) {
  _offset[0] = a;
  _length = 1;
}

inline Offset::Offset(int a, int b) {
  _offset[0] = a;
  _offset[1] = b;
  _length = 2;
}

inline Offset::Offset(int a, int b, int c) {
  _offset[0] = a;
  _offset[1] = b;
  _offset[2] = c;
  _length = 3;
}

inline Offset::Offset(int a, int b, int c, int d) {
  _offset[0] = a;
  _offset[1] = b;
  _offset[2] = c;
  _offset[3] = d;
  _length = 4;
}

inline Offset::Offset(int a, int b, int c, int d, int e) {
  _offset[0] = a;
  _offset[1] = b;
  _offset[2] = c;
  _offset[3] = d;
  _offset[4] = e;
  _length = 5;
}

inline Offset::Offset(const int* offset, int length) {
  if (length > OFFSET_MAX_LENGTH) {
    fprintf(stderr, "Length too large in Offset::Offset(): %d, max is %d", length, OFFSET_MAX_LENGTH);
  }
  for (int i = 0; i < MIN(length, OFFSET_MAX_LENGTH); i += 1) {
    _offset[i] = offset[i];
  }
  _length = length;
}

inline Offset::Offset(const Offset & other) {
  for (int i = 0; i < other._length; i += 1) {
    _offset[i] = other._offset[i];
  }
  _length = other._length;
}

inline int Offset::operator[](const int index) const {
  if (index >= _length) {
    return -1;
  }
  return _offset[index];
}

inline char* Offset::debugString() const {
  // Will be leaked, but don't care during debugging
  char* result = (char*)(malloc(MAX_DEBUG_STRING_LEN));
  switch (_length) {
    case 1:
      snprintf(result, MAX_DEBUG_STRING_LEN, "(%d)",
        _offset[0]);
    break;
    case 2:
      snprintf(result, MAX_DEBUG_STRING_LEN, "(%d, %d)",
        _offset[0], _offset[1]);
    break;
    case 3:
      snprintf(result, MAX_DEBUG_STRING_LEN, "(%d, %d, %d)",
        _offset[0], _offset[1],  _offset[2]);
    break;
    case 4:
      snprintf(result, MAX_DEBUG_STRING_LEN, "(%d, %d, %d, %d)",
        _offset[0], _offset[1],  _offset[2], _offset[3]);
    break;
    case 5:
      snprintf(result, MAX_DEBUG_STRING_LEN, "(%d, %d, %d, %d, %d)",
        _offset[0], _offset[1],  _offset[2], _offset[3], _offset[4]);
    break;
    default:
      snprintf(result, MAX_DEBUG_STRING_LEN, "Bad length in Offset::debugString(): %d", _length);
    break;
  }
  return result;
}

inline bool Offset::operator==(const Offset & other) const {
  if (_length != other._length) {
    return false;
  }
  for (int index = 0; index < _length; index += 1) {
    if (_offset[index] != other._offset[index]) {
      return false;
    }
  }
  return true;
}

#endif // INCLUDE_OFFSET_H
