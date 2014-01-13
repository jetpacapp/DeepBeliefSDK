//
//  binary_format.cpp
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#include "binary_format.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

SBinaryTag* read_tag_from_file(FILE* file) {

  if (file == NULL) {
    fprintf(stderr, "jpcnn read_tag_from_file() NULL file input\n");
    return NULL;
  }

  SBinaryTag tagForSize;
  fread(&tagForSize.type, sizeof(tagForSize.type), 1, file);
  fread(&tagForSize.length, sizeof(tagForSize.length), 1, file);

  const size_t tagTotalBytes = get_total_sizeof_tag(&tagForSize);

  SBinaryTag* result = (SBinaryTag*)(malloc(tagTotalBytes));
  result->type = tagForSize.type;
  result->length = tagForSize.length;

  fread(&result->payload.jpchar[0], 1, result->length, file);

  return result;
}

SBinaryTag* get_tag_from_memory(char* current, const char* end) {
  SBinaryTag* result = (SBinaryTag*)(current);
  const size_t tagTotalBytes = get_total_sizeof_tag(result);
  const size_t remainingLength = (end - current);
  if (remainingLength < tagTotalBytes) {
    fprintf(stderr, "jpcnn get_tag_from_memory() not enough bytes remaining\n");
    return NULL;
  }

  return result;
}

size_t get_total_sizeof_tag(SBinaryTag* tag) {
  return ((2 * sizeof(uint32_t)) + tag->length);
}

SBinaryTag* get_tag_from_dict(SBinaryTag* tag, const char* wantedKey) {
  if (tag->type != JP_DICT) {
    fprintf(stderr, "jpcnn get_tag_from_dict() called on non-DICT for %s\n", wantedKey);
    return NULL;
  }

  SBinaryTag* result = NULL;
  char* current = (char*)(&tag->payload);
  char* end = (current + tag->length);
  while (current < end) {
    SBinaryTag* key = get_tag_from_memory(current, end);
    current += get_total_sizeof_tag(key);
    SBinaryTag* value = get_tag_from_memory(current, end);
    current += get_total_sizeof_tag(value);
    if (strncmp(wantedKey, key->payload.jpchar, key->length) == 0) {
      result = value;
    }
  }

  return result;
}

const char* get_string_from_dict(SBinaryTag* tag, const char* wantedKey) {
  SBinaryTag* valueTag = get_tag_from_dict(tag, wantedKey);
  assert(valueTag != NULL);
  assert(valueTag->type == JP_CHAR);
  return valueTag->payload.jpchar;
}

uint32_t get_uint_from_dict(SBinaryTag* tag, const char* wantedKey) {
  SBinaryTag* valueTag = get_tag_from_dict(tag, wantedKey);
  assert(valueTag != NULL);
  assert(valueTag->type == JP_UINT);
  return valueTag->payload.jpuint;
}

jpfloat_t get_float_from_dict(SBinaryTag* tag, const char* wantedKey) {
  SBinaryTag* valueTag = get_tag_from_dict(tag, wantedKey);
  assert(valueTag != NULL);
  assert(valueTag->type == JP_FL32);
  return valueTag->payload.jpfl32;
}

int count_list_entries(SBinaryTag* tag) {
  assert(tag->type == JP_LIST);
  int result = 0;
  SBinaryTag* current = get_first_list_entry(tag);
  while (current != NULL) {
    result += 1;
    current = get_next_list_entry(tag, current);
  }
  return result;
}

SBinaryTag* get_first_list_entry(SBinaryTag* listTag) {
  assert(listTag->type == JP_LIST);
  SBinaryTag* result = (SBinaryTag*)(listTag->payload.jpchar);
  return result;
}

SBinaryTag* get_next_list_entry(SBinaryTag* listTag, SBinaryTag* previous) {
  assert(listTag->type == JP_LIST);
  char* current = (previous->payload.jpchar + previous->length);
  char* end = (listTag->payload.jpchar + listTag->length);
  if (current >= end) {
    return NULL;
  }
  SBinaryTag* result = get_tag_from_memory(current, end);

  return result;
}

