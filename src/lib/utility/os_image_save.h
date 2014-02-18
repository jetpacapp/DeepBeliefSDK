//
//  os_image_save.h
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#ifndef INCLUDE_OS_IMAGE_SAVE_H
#define INCLUDE_OS_IMAGE_SAVE_H

#ifndef USE_OS_IMAGE
#error "os_image_load.h has been included, but USE_OS_IMAGE was not defined!"
#endif

extern void os_image_save_to_file(const char* filename, unsigned char* data, int width, int height, int channels);

#endif // INCLUDE_OS_IMAGE_SAVE_H
