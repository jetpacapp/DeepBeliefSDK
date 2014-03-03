//
//  cstring_helpers.h
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#ifndef INCLUDE_CSTRING_HELPERS_H
#define INCLUDE_CSTRING_HELPERS_H

// You need to free() the result of this function
char* malloc_and_copy_string(const char* string);

bool string_ends_with(const char* string, const char* suffix);

#endif // INCLUDE_CSTRING_HELPERS_H
