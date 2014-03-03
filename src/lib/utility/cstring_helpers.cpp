//
//  cstring_helpers.cpp
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#include "cstring_helpers.h"

#include <string.h>
#include <stdlib.h>

char* malloc_and_copy_string(const char* input) {
  if (input == NULL) {
    return NULL;
  }
  const size_t inputByteCount = (strlen(input) + 1);
  char* result = (char*)(malloc(inputByteCount));
  strncpy(result, input, inputByteCount);
  return result;
}

bool string_ends_with(const char* string, const char* suffix) {
  if (!string || !suffix) {
    return false;
  }
  size_t stringLength = strlen(string);
  size_t suffixLength = strlen(suffix);
  if (suffixLength >  stringLength) {
    return false;
  }
  const char* stringSuffixStart = (string + (stringLength - suffixLength));
  return (strncmp(stringSuffixStart, suffix, suffixLength) == 0);
}
