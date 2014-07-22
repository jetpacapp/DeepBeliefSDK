//
//  os_image_load.cpp
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#ifdef USE_OS_IMAGE

#include "os_image_load.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ApplicationServices/ApplicationServices.h>

void os_image_save_to_file(const char* filename, unsigned char* data, int width, int height, int channels) {
  assert((channels == 3) || (channels == 4));

  const int bytesPerRow = (width * channels);
  const int bytesPerImage = (bytesPerRow * height);
  const int bitsPerChannel = 8;
  const int bitsPerPixel = (bitsPerChannel * channels);

  CGColorSpaceRef colorspace = CGColorSpaceCreateDeviceRGB();
  CFDataRef rgbData = CFDataCreate(NULL, data, bytesPerImage);
  CGDataProviderRef provider = CGDataProviderCreateWithCFData(rgbData);
  CGImageRef imageRef = CGImageCreate(
    width,
    height,
    bitsPerChannel,
    bitsPerPixel,
    bytesPerRow,
    colorspace,
    kCGBitmapByteOrderDefault,
    provider,
    NULL,
    true,
    kCGRenderingIntentDefault);
  CFRelease(rgbData);
  CGDataProviderRelease(provider);
  CGColorSpaceRelease(colorspace);

  CFURLRef url = CFURLCreateFromFileSystemRepresentation(NULL, (const uint8_t*)filename, (CFIndex)strlen(filename), false);
  CGImageDestinationRef destination = CGImageDestinationCreateWithURL(url, kUTTypePNG, 1, NULL);
  CGImageDestinationAddImage(destination, imageRef, nil);

  if (!CGImageDestinationFinalize(destination)) {
    fprintf(stderr, "Failed to write image to %s\n", filename);
  }

  CFRelease(destination);
  CFRelease(url);
  CGImageRelease(imageRef);
}

#endif // USE_OS_IMAGE