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
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

SBinaryTag* read_tag_from_file(const char* filename, bool useMemoryMap) {

  if (filename == NULL) {
    fprintf(stderr, "jpcnn read_tag_from_file() NULL filename input\n");
    return NULL;
  }

  SBinaryTag* result;
  if (useMemoryMap) {
    const int fileHandle = open(filename, O_RDONLY);
    struct stat statBuffer;
    fstat(fileHandle, &statBuffer);
    const size_t bytesInFile = (size_t)(statBuffer.st_size);
    result = (SBinaryTag*)(mmap(NULL, bytesInFile, PROT_READ, MAP_SHARED, fileHandle, 0));
  } else {
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
      fprintf(stderr, "read_tag_from_file() - couldn't open '%s'\n", filename);
      return NULL;
    }
    SBinaryTag tagForSize;
    fread(&tagForSize.type, sizeof(tagForSize.type), 1, file);
    fread(&tagForSize.length, sizeof(tagForSize.length), 1, file);

    const size_t tagTotalBytes = get_total_sizeof_tag(&tagForSize);

    result = (SBinaryTag*)(malloc(tagTotalBytes));
    result->type = tagForSize.type;
    result->length = tagForSize.length;

    fread(&result->payload.jpchar[0], 1, result->length, file);
    fclose(file);
  }

  return result;
}

void deallocate_file_tag(SBinaryTag* fileTag, bool useMemoryMap) {
  if (useMemoryMap) {
    // This assumes that there's only a single root tag in a binary file, so we can
    // calculate the file length based on the tag length.
    const size_t tagTotalBytes = get_total_sizeof_tag(fileTag);
    munmap(fileTag, tagTotalBytes);
  } else {
    free(fileTag);
  }
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

SBinaryTag* create_list_tag() {
  SBinaryTag* result = (SBinaryTag*)(malloc(2 * sizeof(uint32_t)));
  result->type = JP_LIST;
  result->length = 0;
  return result;
}

SBinaryTag* add_tag_to_list(SBinaryTag* tag, SBinaryTag* subTag) {
  const size_t oldLength = tag->length;
  const size_t subTagSize = get_total_sizeof_tag(subTag);
  const size_t newLength = (oldLength + subTagSize);
  const size_t newTagSize = (get_total_sizeof_tag(tag) + subTagSize);
  SBinaryTag* result = (SBinaryTag*)(realloc(tag, newTagSize));
  char* newTagDestination = ((char*)(result)) + (8 + oldLength);
  memcpy(newTagDestination, subTag, subTagSize);
  result->length = (int)(newLength);
  return result;
}

SBinaryTag* create_dict_tag() {
  SBinaryTag* result = (SBinaryTag*)(malloc(2 * sizeof(uint32_t)));
  result->type = JP_DICT;
  result->length = 0;
  return result;
}

SBinaryTag* add_tag_to_dict(SBinaryTag* tag, const char* key, SBinaryTag* valueTag) {
  SBinaryTag* result = tag;
  SBinaryTag* keyTag = create_string_tag(key);
  result = add_tag_to_list(result, keyTag);
  free(keyTag);
  result = add_tag_to_list(result, valueTag);
  return result;
}

SBinaryTag* create_string_tag(const char* value) {
  const size_t stringLength = strlen(value);
  const size_t bufferLength = ((((stringLength + 1) + 3) / 4) * 4);
  SBinaryTag* result = (SBinaryTag*)(malloc((2 * sizeof(uint32_t)) + bufferLength));
  result->type = JP_CHAR;
  result->length = (int)(bufferLength);
  char* valueDestination = (((char*)(result)) + 8);
  memcpy(valueDestination, value, stringLength);
  char* terminatorDestination = (((char*)(result)) + 8 + stringLength);
  const size_t terminatorCount = (bufferLength - stringLength);
  memset(terminatorDestination, 0, terminatorCount);
  return result;
}

SBinaryTag* create_uint_tag(uint32_t value) {
  SBinaryTag* result = (SBinaryTag*)(malloc(8 + sizeof(uint32_t)));
  result->type = JP_UINT;
  result->length = sizeof(uint32_t);
  result->payload.jpuint = value;
  return result;
}

SBinaryTag* create_float_tag(float value) {
  SBinaryTag* result = (SBinaryTag*)(malloc(8 + sizeof(float)));
  result->type = JP_FL32;
  result->length = sizeof(float);
  result->payload.jpfl32 = value;
  return result;
}

SBinaryTag* create_float_array_tag(float* value, int elementCount) {
  const size_t bufferLength = (elementCount * sizeof(float));
  SBinaryTag* result = (SBinaryTag*)(malloc((2 * sizeof(uint32_t)) + bufferLength));
  result->type = JP_FARY;
  result->length = (int)(bufferLength);
  char* valueDestination = (((char*)(result)) + 8);
  memcpy(valueDestination, value, bufferLength);
  return result;
}

SBinaryTag* create_blob_tag(void* value, int sizeofValue) {
  const size_t bufferLength = (((sizeofValue + 3) / 4) * 4);
  SBinaryTag* result = (SBinaryTag*)(malloc((2 * sizeof(uint32_t)) + bufferLength));
  result->type = JP_BLOB;
  result->length = (int)(bufferLength);
  char* valueDestination = (((char*)(result)) + 8);
  memcpy(valueDestination, value, sizeofValue);
  memset((valueDestination + sizeofValue), 0, (bufferLength - sizeofValue));
  return result;
}

SBinaryTag* add_string_to_dict(SBinaryTag* tag, const char* key, const char* value) {
  SBinaryTag* valueTag = create_string_tag(value);
  SBinaryTag* result = add_tag_to_dict(tag, key, valueTag);
  free(valueTag);
  return result;
}

SBinaryTag* add_uint_to_dict(SBinaryTag* tag, const char* key, uint32_t value) {
  SBinaryTag* valueTag = create_uint_tag(value);
  SBinaryTag* result = add_tag_to_dict(tag, key, valueTag);
  free(valueTag);
  return result;
}

SBinaryTag* add_float_to_dict(SBinaryTag* tag, const char* key, float value) {
  SBinaryTag* valueTag = create_float_tag(value);
  SBinaryTag* result = add_tag_to_dict(tag, key, valueTag);
  free(valueTag);
  return result;
}

SBinaryTag* add_float_array_to_dict(SBinaryTag* tag, const char* key, float* value, int elementCount) {
  SBinaryTag* valueTag = create_float_array_tag(value, elementCount);
  SBinaryTag* result = add_tag_to_dict(tag, key, valueTag);
  free(valueTag);
  return result;
}

SBinaryTag* add_blob_to_dict(SBinaryTag* tag, const char* key, void* value, int sizeofValue) {
  SBinaryTag* valueTag = create_blob_tag(value, sizeofValue);
  SBinaryTag* result = add_tag_to_dict(tag, key, valueTag);
  free(valueTag);
  return result;
}

SBinaryTag* add_string_to_list(SBinaryTag* tag, const char* value) {
  SBinaryTag* valueTag = create_string_tag(value);
  SBinaryTag* result = add_tag_to_list(tag, valueTag);
  free(valueTag);
  return result;
}

SBinaryTag* add_uint_to_list(SBinaryTag* tag, uint32_t value) {
  SBinaryTag* valueTag = create_uint_tag(value);
  SBinaryTag* result = add_tag_to_list(tag, valueTag);
  free(valueTag);
  return result;
}

SBinaryTag* add_float_to_list(SBinaryTag* tag, float value) {
  SBinaryTag* valueTag = create_float_tag(value);
  SBinaryTag* result = add_tag_to_list(tag, valueTag);
  free(valueTag);
  return result;
}

SBinaryTag* add_float_array_to_list(SBinaryTag* tag, float* value, int elementCount) {
  SBinaryTag* valueTag = create_float_array_tag(value, elementCount);
  SBinaryTag* result = add_tag_to_list(tag, valueTag);
  free(valueTag);
  return result;
}

SBinaryTag* add_blob_to_list(SBinaryTag* tag, void* value, int sizeofValue) {
  SBinaryTag* valueTag = create_blob_tag(value, sizeofValue);
  SBinaryTag* result = add_tag_to_list(tag, valueTag);
  free(valueTag);
  return result;
}
