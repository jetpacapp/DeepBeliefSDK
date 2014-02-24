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

unsigned char* os_image_load_from_file(const char* filename, int* outWidth, int* outHeight, int* outChannels, int unused) {
  const int fileHandle = open(filename, O_RDONLY);
  struct stat statBuffer;
  fstat(fileHandle, &statBuffer);
  const size_t bytesInFile = (size_t)(statBuffer.st_size);
  uint8_t* fileData = (uint8_t*)(mmap(NULL, bytesInFile, PROT_READ, MAP_SHARED, fileHandle, 0));
  if (fileData == MAP_FAILED) {
    fprintf(stderr, "Couldn't open file '%s' with mmap\n", filename);
    return NULL;
  }
  CFDataRef fileDataRef = CFDataCreateWithBytesNoCopy(NULL, fileData, bytesInFile, kCFAllocatorNull);
  CGDataProviderRef imageProvider = CGDataProviderCreateWithCFData(fileDataRef);

  const char* suffix = strrchr(filename, '.');
  if (!suffix || suffix == filename) {
    suffix = "";
  }
  CGImageRef image;
  if (strcasecmp(suffix, ".png") == 0) {
    image = CGImageCreateWithPNGDataProvider(imageProvider, NULL, true, kCGRenderingIntentDefault);
  } else if ((strcasecmp(suffix, ".jpg") == 0) ||
    (strcasecmp(suffix, ".jpeg") == 0)) {
    image = CGImageCreateWithJPEGDataProvider(imageProvider, NULL, true, kCGRenderingIntentDefault);
  } else {
    munmap(fileData, bytesInFile);
    close(fileHandle);
    CFRelease(imageProvider);
    CFRelease(fileDataRef);
    fprintf(stderr, "Unknown suffix for file '%s'\n", filename);
    return NULL;
  }

  const int width = (int)CGImageGetWidth(image);
  const int height = (int)CGImageGetHeight(image);
  const int channels = 4;
  CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
  const int bytesPerRow = (width * channels);
  const int bytesInImage = (bytesPerRow * height);
  uint8_t* result = (uint8_t*)(malloc(bytesInImage));
  const int bitsPerComponent = 8;
  CGContextRef context = CGBitmapContextCreate(result, width, height,
    bitsPerComponent, bytesPerRow, colorSpace,
    kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big);
  CGColorSpaceRelease(colorSpace);
  CGContextDrawImage(context, CGRectMake(0, 0, width, height), image);
  CGContextRelease(context);
  CFRelease(image);

  munmap(fileData, bytesInFile);
  close(fileHandle);
  CFRelease(imageProvider);
  CFRelease(fileDataRef);

  *outWidth = width;
  *outHeight = height;
  *outChannels = channels;

  return result;
}

void os_image_free(unsigned char* data) {
  free(data);
}

#endif // USE_OS_IMAGE