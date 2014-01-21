//
//  binary_format.h
//  jpcnn
//
//  This simple binary format is:
// <tag type (4 bytes)>
// <payload length (4 bytes)>
// <payload (payload length bytes>
// <type><length><payload><type><....
//
//  It's fairly flexible, unknown types can be skipped, and it's easy to read
//  without having to get too fancy with strings and arrays.
//
//  Types are:
// 'CHAR' - ASCII string, null-terminated
// 'UINT' - 32-bit integer
// 'FL32' - 32 bit float
// 'FARY' - An array of 32 bit floats
// 'DICT' - A sequence of (<CHAR><tag>) pairs
// 'LIST' - A sequence of arbitrary tags
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#ifndef INCLUDE_BINARY_FORMAT_H
#define INCLUDE_BINARY_FORMAT_H

#include <stdio.h>
#include <stdint.h>

#include "jpcnn.h"

#define JP_CHAR (0x52414843) // 'CHAR'
#define JP_UINT (0x544E4955) // 'UINT'
#define JP_FL32 (0x32334C46) // 'FL32'
#define JP_FARY (0x59524146) // 'FARY'
#define JP_DICT (0x54434944) // 'DICT'
#define JP_LIST (0x5453494C) // 'LIST'

typedef struct SBinaryTagStruct {
  uint32_t type;
  uint32_t length;
  union {
    uint32_t jpuint;
    float jpfl32;
    char jpchar[1];
    float jpfary[1];
  } payload;
} SBinaryTag;

// read_tag_from_file() requires the caller to call deallocate_file_tag
// on the result.
// All other functions work in-place and don't need any memory management.
SBinaryTag* read_tag_from_file(const char* filename, bool useMemoryMap);
void deallocate_file_tag(SBinaryTag* fileTag, bool useMemoryMap);

SBinaryTag* get_tag_from_memory(char* current, const char* end);
size_t get_total_sizeof_tag(SBinaryTag* tag);
SBinaryTag* get_tag_from_dict(SBinaryTag* tag, const char* wantedKey);
const char* get_string_from_dict(SBinaryTag* tag, const char* wantedKey);
uint32_t get_uint_from_dict(SBinaryTag* tag, const char* wantedKey);
jpfloat_t get_float_from_dict(SBinaryTag* tag, const char* wantedKey);

int count_list_entries(SBinaryTag* tag);
SBinaryTag* get_first_list_entry(SBinaryTag* listTag);
SBinaryTag* get_next_list_entry(SBinaryTag* listTag, SBinaryTag* previous);

#endif // INCLUDE_BINARY_FORMAT_H
