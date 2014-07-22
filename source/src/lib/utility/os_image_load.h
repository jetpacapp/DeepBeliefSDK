//
//  os_image_load.h
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#ifndef INCLUDE_OS_IMAGE_LOAD_H
#define INCLUDE_OS_IMAGE_LOAD_H

#ifndef USE_OS_IMAGE
#error "os_image_load.h has been included, but USE_OS_IMAGE was not defined!"
#endif

extern unsigned char* os_image_load_from_file(const char* filename, int* outWidth, int* outHeight, int* outChannels, int unused);
extern void os_image_free(unsigned char* data);

#endif // INCLUDE_OS_IMAGE_LOAD_H
