//
//  buffer.h
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#ifndef INCLUDE_BUFFER_H
#define INCLUDE_BUFFER_H

#include "jpcnn.h"
#include "dimensions.h"

class Buffer
{
public:

  Buffer(const Dimensions& dims);
  Buffer(const Dimensions& dims, jpfloat_t* data);
  virtual ~Buffer();

  Dimensions _dims;
  jpfloat_t* _data;
  bool _doesOwnData;
  char* _debugString;
  char* _name;

  jpfloat_t* dataEnd();
  char* debugString();
  void saveDebugImage();
  void setName(const char*);
};

inline jpfloat_t* Buffer::dataEnd() { return (_data + _dims.elementCount()); }

extern Buffer* buffer_from_image_file(const char* filename);
extern Buffer* buffer_from_dump_file(const char* filename);
extern void buffer_save_to_image_file(Buffer* buffer, const char* filename);
extern bool buffer_are_all_close(Buffer* a, Buffer* b, jpfloat_t tolerance=0.000001);
extern Buffer* buffer_view_at_top_index(Buffer* input, int index);

#endif // INCLUDE_BUFFER_H
