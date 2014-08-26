//
//  glgemm.cpp
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#ifdef USE_OPENGL

#include "glgemm.h"

#include <assert.h>
#include <string.h>
#include <math.h>

#include "glcontext.h"
#include "glbuffer.h"
#include "glprogram.h"
#include "dimensions.h"
#include "buffer.h"
#include "matrix_ops.h"

static Dimensions* physicalFromVirtualSize(Dimensions* virtualSize, bool doResize, int bitsPerElement = 32);

static const char* g_gemmVertexShader = "                       \n\
  uniform mat4 modelViewProjectionMatrix;                       \n\
  attribute vec2 aVertexPosition;                               \n\
  attribute vec2 aTexCoord;                                     \n\
  varying vec2 outTexCoord;                                     \n\
                                                                \n\
  void main(void) {                                             \n\
    gl_Position = modelViewProjectionMatrix * vec4(aVertexPosition, 0.0, 1.0); \n\
    outTexCoord = aTexCoord;                                    \n\
  }                                                             \n\
";

static const char* g_gemmFragmentShader = "                     \n\
  varying vec2 outTexCoord;                                     \n\
  uniform sampler2D a;                                          \n\
  uniform vec2 aXTransform;                                     \n\
  uniform vec2 aYTransform;                                     \n\
  uniform vec2 aVirtualSize;                                    \n\
  uniform vec2 aRecipVirtualSize;                               \n\
  uniform vec2 aPhysicalSize;                                   \n\
  uniform vec2 aRecipPhysicalSize;                              \n\
  uniform sampler2D b;                                          \n\
  uniform vec2 bVirtualSize;                                    \n\
  uniform vec2 bRecipVirtualSize;                               \n\
  uniform vec2 bPhysicalSize;                                   \n\
  uniform vec2 bRecipPhysicalSize;                              \n\
  uniform sampler2D c;                                          \n\
  uniform vec2 cVirtualSize;                                    \n\
  uniform vec2 cRecipVirtualSize;                               \n\
  uniform vec2 cPhysicalSize;                                   \n\
  uniform vec2 cRecipPhysicalSize;                              \n\
  uniform float alpha;                                          \n\
  uniform float beta;                                           \n\
  uniform int k;                                                \n\
  float shiftRight(float v, float amt) {                        \n\
    v = floor(v) + 0.5;                                         \n\
    return floor(v / exp2(amt));                                \n\
  }                                                             \n\
  float shiftLeft(float v, float amt) {                         \n\
    return floor(v * exp2(amt) + 0.5);                          \n\
  }                                                             \n\
  float maskLast(float v, float bits) {                         \n\
    return mod(v, shiftLeft(1.0, bits));                        \n\
  }                                                             \n\
  float extractBits(float num, float from, float to) {          \n\
    from = floor(from + 0.5);                                   \n\
    to = floor(to + 0.5);                                       \n\
    return maskLast(shiftRight(num, from), to - from);          \n\
  }                                                             \n\
  vec4 encode32(float val) {                                    \n\
    if (val == 0.0)                                             \n\
      return vec4(0, 0, 0, 0);                                  \n\
    float sign = val > 0.0 ? 0.0 : 1.0;                         \n\
    val = abs(val);                                             \n\
    float exponent = floor(log2(val));                          \n\
    float biased_exponent = exponent + 127.0;                   \n\
    float fraction = ((val / exp2(exponent)) - 1.0) * 8388608.0; \n\
    float t = biased_exponent / 2.0;                            \n\
    float last_bit_of_biased_exponent = fract(t) * 2.0;         \n\
    float remaining_bits_of_biased_exponent = floor(t);         \n\
    float byte4 = extractBits(fraction, 0.0, 8.0) / 255.0;      \n\
    float byte3 = extractBits(fraction, 8.0, 16.0) / 255.0;     \n\
    float byte2 = (last_bit_of_biased_exponent * 128.0 + extractBits(fraction, 16.0, 23.0)) / 255.0; \n\
    float byte1 = (sign * 128.0 + remaining_bits_of_biased_exponent) / 255.0; \n\
    return vec4(byte4, byte3, byte2, byte1);                      \n\
  }                                                             \n\
  float decode32(vec4 rgba) {                                   \n\
    float byte1 = rgba[3] * 255.0;                              \n\
    float byte2 = rgba[2] * 255.0;                              \n\
    float byte3 = rgba[1] * 255.0;                              \n\
    float byte4 = rgba[0] * 255.0;                              \n\
    float Sign = 1.0 - step(128.0, byte1)*2.0;                 \n\
    float Exponent = 2.0 * mod(byte1,128.0) + step(128.0, byte2) - 127.0; \n\
    float Mantissa = mod(byte2, 128.0) * 65536.0 + byte3 * 256.0 + byte4 + float(0x800000); \n\
    float Result =  Sign * exp2(Exponent) * (Mantissa * exp2(-23.0 )); \n\
    return Result;                                     \n\
  }                                                             \n\
  vec2 virtualToPhysicalCoords(vec2 normalizedVirtualCoords, vec2 virtualSize, vec2 physicalSize, vec2 recipPhysicalSize) { \n\
    vec2 virtualCoords = (normalizedVirtualCoords * virtualSize); \n\
    float elementOffset = ((floor(virtualCoords.y) * virtualSize.x) + floor(virtualCoords.x));\n\
    vec2 physicalCoords;                                        \n\
    physicalCoords.x = (mod(elementOffset, physicalSize.x) + 0.5);      \n\
    physicalCoords.y = (floor((elementOffset + 0.5) * recipPhysicalSize.x) + 0.5); \n\
    vec2 normalizedPhysicalCoords = (physicalCoords * recipPhysicalSize); \n\
    return normalizedPhysicalCoords;                            \n\
  }                                                             \n\
  vec2 physicalToVirtualCoords(vec2 normalizedPhysicalCoords, vec2 virtualSize, vec2 recipVirtualSize, vec2 physicalSize) { \n\
    vec2 physicalCoords = (normalizedPhysicalCoords * physicalSize); \n\
    float elementOffset = ((floor(physicalCoords.y) * physicalSize.x) + floor(physicalCoords.x));\n\
    vec2 virtualCoords;                                        \n\
    virtualCoords.x = (mod(elementOffset, virtualSize.x) + 0.5);       \n\
    virtualCoords.y = (floor((elementOffset + 0.5) * recipVirtualSize.x) + 0.5); \n\
    vec2 normalizedVirtualCoords = (virtualCoords * recipVirtualSize); \n\
    return normalizedVirtualCoords;                            \n\
  }                                                             \n\
  void main(void) {                                             \n\
    vec2 cPhysicalCoords = (outTexCoord * cRecipPhysicalSize); \n\
    vec2 texCoord = physicalToVirtualCoords(cPhysicalCoords, cVirtualSize, cRecipVirtualSize, cPhysicalSize); \n\
    float i = (texCoord.x * cVirtualSize.x);                    \n\
    float j = (texCoord.y * cVirtualSize.y);                    \n\
    float cValue;                                               \n\
    if (beta != 0.0) {                                          \n\
      cValue = decode32(texture2D(c, cPhysicalCoords));             \n\
    } else {                                                    \n\
      cValue = 0.0;                                             \n\
    }                                                           \n\
    float total = 0.0;                                          \n\
    for (int l = 0; l < 24; l += 1) {                            \n\
      float lCoord = (float(l) + 0.5);                          \n\
      vec2 aInputCoords = vec2(i, lCoord);                      \n\
      vec2 aTransformedCoords = vec2(dot(aInputCoords, aXTransform), dot(aInputCoords, aYTransform)); \n\
      vec2 aVirtualCoords = (aTransformedCoords * aRecipVirtualSize); \n\
      vec2 aPhysicalCoords = virtualToPhysicalCoords(aVirtualCoords, aVirtualSize, aPhysicalSize, aRecipPhysicalSize); \n\
      float aValue = decode32(texture2D(a, aPhysicalCoords));    \n\
      vec2 bVirtualCoords = (vec2(lCoord, j) * bRecipVirtualSize); \n\
      vec2 bPhysicalCoords = virtualToPhysicalCoords(bVirtualCoords, bVirtualSize, bPhysicalSize, bRecipPhysicalSize); \n\
      float bValue = decode32(texture2D(b, bPhysicalCoords));   \n\
      total += (aValue * bValue);                               \n\
      if (l >= k) {                                             \n\
        break;                                                  \n\
      }                                                         \n\
    }                                                           \n\
    gl_FragColor = encode32((alpha * total) + (beta * cValue)); \n\
  }                                                             \n\
";

static const char* g_gemmFragmentShader16Bit = "                     \n\
  varying vec2 outTexCoord;                                     \n\
  uniform sampler2D a;                                          \n\
  uniform vec2 aXTransform;                                     \n\
  uniform vec2 aYTransform;                                     \n\
  uniform vec2 aVirtualSize;                                    \n\
  uniform vec2 aRecipVirtualSize;                               \n\
  uniform vec2 aPhysicalSize;                                   \n\
  uniform vec2 aRecipPhysicalSize;                              \n\
  uniform float aMin;                                           \n\
  uniform float aRange;                                         \n\
  uniform sampler2D b;                                          \n\
  uniform vec2 bVirtualSize;                                    \n\
  uniform vec2 bRecipVirtualSize;                               \n\
  uniform vec2 bPhysicalSize;                                   \n\
  uniform vec2 bRecipPhysicalSize;                              \n\
  uniform sampler2D c;                                          \n\
  uniform vec2 cVirtualSize;                                    \n\
  uniform vec2 cRecipVirtualSize;                               \n\
  uniform vec2 cPhysicalSize;                                   \n\
  uniform vec2 cRecipPhysicalSize;                              \n\
  uniform float alpha;                                          \n\
  uniform float beta;                                           \n\
  uniform int k;                                                \n\
  float shiftRight(float v, float amt) {                        \n\
    v = floor(v) + 0.5;                                         \n\
    return floor(v / exp2(amt));                                \n\
  }                                                             \n\
  float shiftLeft(float v, float amt) {                         \n\
    return floor(v * exp2(amt) + 0.5);                          \n\
  }                                                             \n\
  float maskLast(float v, float bits) {                         \n\
    return mod(v, shiftLeft(1.0, bits));                        \n\
  }                                                             \n\
  float extractBits(float num, float from, float to) {          \n\
    from = floor(from + 0.5);                                   \n\
    to = floor(to + 0.5);                                       \n\
    return maskLast(shiftRight(num, from), to - from);          \n\
  }                                                             \n\
  vec4 encode32(float val) {                                    \n\
    if (val == 0.0)                                             \n\
      return vec4(0, 0, 0, 0);                                  \n\
    float sign = val > 0.0 ? 0.0 : 1.0;                         \n\
    val = abs(val);                                             \n\
    float exponent = floor(log2(val));                          \n\
    float biased_exponent = exponent + 127.0;                   \n\
    float fraction = ((val / exp2(exponent)) - 1.0) * 8388608.0; \n\
    float t = biased_exponent / 2.0;                            \n\
    float last_bit_of_biased_exponent = fract(t) * 2.0;         \n\
    float remaining_bits_of_biased_exponent = floor(t);         \n\
    float byte4 = extractBits(fraction, 0.0, 8.0) / 255.0;      \n\
    float byte3 = extractBits(fraction, 8.0, 16.0) / 255.0;     \n\
    float byte2 = (last_bit_of_biased_exponent * 128.0 + extractBits(fraction, 16.0, 23.0)) / 255.0; \n\
    float byte1 = (sign * 128.0 + remaining_bits_of_biased_exponent) / 255.0; \n\
    return vec4(byte4, byte3, byte2, byte1);                      \n\
  }                                                             \n\
  float decode32(vec4 rgba) {                                   \n\
    float byte1 = rgba[3] * 255.0;                              \n\
    float byte2 = rgba[2] * 255.0;                              \n\
    float byte3 = rgba[1] * 255.0;                              \n\
    float byte4 = rgba[0] * 255.0;                              \n\
    float Sign = 1.0 - step(128.0, byte1)*2.0;                 \n\
    float Exponent = 2.0 * mod(byte1,128.0) + step(128.0, byte2) - 127.0; \n\
    float Mantissa = mod(byte2, 128.0) * 65536.0 + byte3 * 256.0 + byte4 + float(0x800000); \n\
    float Result =  Sign * exp2(Exponent) * (Mantissa * exp2(-23.0 )); \n\
    return Result;                                              \n\
  }                                                             \n\
  float decode16(vec4 rgba, int component, float min, float range) { \n\
    float byte0;                                                \n\
    float byte1;                                                \n\
    if (component == 0) {                                       \n\
      byte0 = rgba[0];                                          \n\
      byte1 = rgba[1];                                          \n\
    } else {                                                    \n\
      byte0 = rgba[2];                                          \n\
      byte1 = rgba[3];                                          \n\
    }                                                           \n\
    float value = (byte1 * (255.0 / 256.0)) + (byte0 / 256.0);  \n\
    float result = ((value * range) + min);                     \n\
    return result;                                              \n\
  }                                                             \n\
  vec2 virtualToPhysicalCoords(vec2 normalizedVirtualCoords, vec2 virtualSize, vec2 physicalSize, vec2 recipPhysicalSize) { \n\
    vec2 virtualCoords = (normalizedVirtualCoords * virtualSize); \n\
    float elementOffset = ((floor(virtualCoords.y) * virtualSize.x) + floor(virtualCoords.x));\n\
    vec2 physicalCoords;                                        \n\
    physicalCoords.x = (mod(elementOffset, physicalSize.x) + 0.5);      \n\
    physicalCoords.y = (floor((elementOffset + 0.5) * recipPhysicalSize.x) + 0.5); \n\
    vec2 normalizedPhysicalCoords = (physicalCoords * recipPhysicalSize); \n\
    return normalizedPhysicalCoords;                            \n\
  }                                                             \n\
  vec2 virtualToPhysicalCoordsFixed(vec2 normalizedVirtualCoords, vec2 virtualSize, vec2 physicalSize, vec2 recipPhysicalSize, float elementsPerPixelScale) { \n\
    vec2 virtualCoords = (normalizedVirtualCoords * virtualSize); \n\
    float elementOffset = ((floor(virtualCoords.y) * virtualSize.x * elementsPerPixelScale) + floor(virtualCoords.x * elementsPerPixelScale));\n\
    vec2 physicalCoords;                                        \n\
    physicalCoords.x = (mod(elementOffset, physicalSize.x) + 0.5);      \n\
    physicalCoords.y = (floor((elementOffset + 0.5) * recipPhysicalSize.x) + 0.5); \n\
    vec2 normalizedPhysicalCoords = (physicalCoords * recipPhysicalSize); \n\
    return normalizedPhysicalCoords;                            \n\
  }                                                             \n\
  vec2 physicalToVirtualCoords(vec2 normalizedPhysicalCoords, vec2 virtualSize, vec2 recipVirtualSize, vec2 physicalSize) { \n\
    vec2 physicalCoords = (normalizedPhysicalCoords * physicalSize); \n\
    float elementOffset = ((floor(physicalCoords.y) * physicalSize.x) + floor(physicalCoords.x));\n\
    vec2 virtualCoords;                                        \n\
    virtualCoords.x = (mod(elementOffset, virtualSize.x) + 0.5);       \n\
    virtualCoords.y = (floor((elementOffset + 0.5) * recipVirtualSize.x) + 0.5); \n\
    vec2 normalizedVirtualCoords = (virtualCoords * recipVirtualSize); \n\
    return normalizedVirtualCoords;                            \n\
  }                                                             \n\
  void main(void) {                                             \n\
    vec2 cPhysicalCoords = (outTexCoord * cRecipPhysicalSize); \n\
    vec2 texCoord = physicalToVirtualCoords(cPhysicalCoords, cVirtualSize, cRecipVirtualSize, cPhysicalSize); \n\
    float i = (texCoord.x * cVirtualSize.x);                    \n\
    float j = (texCoord.y * cVirtualSize.y);                    \n\
    float cValue;                                               \n\
    if (beta != 0.0) {                                          \n\
      cValue = decode32(texture2D(c, cPhysicalCoords));             \n\
    } else {                                                    \n\
      cValue = 0.0;                                             \n\
    }                                                           \n\
    float total = 0.0;                                          \n\
    for (int l = 0; l < 24; l += 1) {                           \n\
      float lCoord = (float(l) + 0.5);                          \n\
      vec2 aInputCoords = vec2(i, lCoord);                      \n\
      vec2 aTransformedCoords = vec2(dot(aInputCoords, aXTransform), dot(aInputCoords, aYTransform)); \n\
      int aComponent = int(mod(floor(aTransformedCoords.x), 2.0)); \n\
      vec2 aVirtualCoords = (aTransformedCoords * aRecipVirtualSize); \n\
      vec2 aPhysicalCoords = virtualToPhysicalCoordsFixed(aVirtualCoords, aVirtualSize, aPhysicalSize, aRecipPhysicalSize, 0.5); \n\
      float aValue = decode16(texture2D(a, aPhysicalCoords), aComponent, aMin, aRange);    \n\
      vec2 bVirtualCoords = (vec2(lCoord, j) * bRecipVirtualSize); \n\
      vec2 bPhysicalCoords = virtualToPhysicalCoords(bVirtualCoords, bVirtualSize, bPhysicalSize, bRecipPhysicalSize); \n\
      float bValue = decode32(texture2D(b, bPhysicalCoords));   \n\
      total += (aValue * bValue);                               \n\
      if (l >= k) {                                             \n\
        break;                                                  \n\
      }                                                         \n\
    }                                                           \n\
    gl_FragColor = encode32((alpha * total) + (beta * cValue)); \n\
  }                                                             \n\
";

static const char* g_gemmFragmentShader8Bit = "                     \n\
  varying vec2 outTexCoord;                                     \n\
  uniform sampler2D a;                                          \n\
  uniform vec2 aXTransform;                                     \n\
  uniform vec2 aYTransform;                                     \n\
  uniform vec2 aVirtualSize;                                    \n\
  uniform vec2 aRecipVirtualSize;                               \n\
  uniform vec2 aPhysicalSize;                                   \n\
  uniform vec2 aRecipPhysicalSize;                              \n\
  uniform float aMin;                                           \n\
  uniform float aRange;                                         \n\
  uniform sampler2D b;                                          \n\
  uniform vec2 bVirtualSize;                                    \n\
  uniform vec2 bRecipVirtualSize;                               \n\
  uniform vec2 bPhysicalSize;                                   \n\
  uniform vec2 bRecipPhysicalSize;                              \n\
  uniform sampler2D c;                                          \n\
  uniform vec2 cVirtualSize;                                    \n\
  uniform vec2 cRecipVirtualSize;                               \n\
  uniform vec2 cPhysicalSize;                                   \n\
  uniform vec2 cRecipPhysicalSize;                              \n\
  uniform float alpha;                                          \n\
  uniform float beta;                                           \n\
  uniform int k;                                                \n\
  float shiftRight(float v, float amt) {                        \n\
    v = floor(v) + 0.5;                                         \n\
    return floor(v / exp2(amt));                                \n\
  }                                                             \n\
  float shiftLeft(float v, float amt) {                         \n\
    return floor(v * exp2(amt) + 0.5);                          \n\
  }                                                             \n\
  float maskLast(float v, float bits) {                         \n\
    return mod(v, shiftLeft(1.0, bits));                        \n\
  }                                                             \n\
  float extractBits(float num, float from, float to) {          \n\
    from = floor(from + 0.5);                                   \n\
    to = floor(to + 0.5);                                       \n\
    return maskLast(shiftRight(num, from), to - from);          \n\
  }                                                             \n\
  vec4 encode32(float val) {                                    \n\
    if (val == 0.0)                                             \n\
      return vec4(0, 0, 0, 0);                                  \n\
    float sign = val > 0.0 ? 0.0 : 1.0;                         \n\
    val = abs(val);                                             \n\
    float exponent = floor(log2(val));                          \n\
    float biased_exponent = exponent + 127.0;                   \n\
    float fraction = ((val / exp2(exponent)) - 1.0) * 8388608.0; \n\
    float t = biased_exponent / 2.0;                            \n\
    float last_bit_of_biased_exponent = fract(t) * 2.0;         \n\
    float remaining_bits_of_biased_exponent = floor(t);         \n\
    float byte4 = extractBits(fraction, 0.0, 8.0) / 255.0;      \n\
    float byte3 = extractBits(fraction, 8.0, 16.0) / 255.0;     \n\
    float byte2 = (last_bit_of_biased_exponent * 128.0 + extractBits(fraction, 16.0, 23.0)) / 255.0; \n\
    float byte1 = (sign * 128.0 + remaining_bits_of_biased_exponent) / 255.0; \n\
    return vec4(byte4, byte3, byte2, byte1);                      \n\
  }                                                             \n\
  float decode32(vec4 rgba) {                                   \n\
    float byte1 = rgba[3] * 255.0;                              \n\
    float byte2 = rgba[2] * 255.0;                              \n\
    float byte3 = rgba[1] * 255.0;                              \n\
    float byte4 = rgba[0] * 255.0;                              \n\
    float Sign = 1.0 - step(128.0, byte1)*2.0;                 \n\
    float Exponent = 2.0 * mod(byte1,128.0) + step(128.0, byte2) - 127.0; \n\
    float Mantissa = mod(byte2, 128.0) * 65536.0 + byte3 * 256.0 + byte4 + float(0x800000); \n\
    float Result =  Sign * exp2(Exponent) * (Mantissa * exp2(-23.0 )); \n\
    return Result;                                     \n\
  }                                                             \n\
  float decode8(vec4 rgba, int component, float min, float range) { \n\
    float value;                                                \n\
    if (component == 0) {                                       \n\
      value = rgba[0];                                          \n\
    } else if (component == 1) {                                \n\
      value = rgba[1];                                          \n\
    } else if (component == 2) {                                \n\
      value = rgba[2];                                          \n\
    } else {                                                    \n\
      value = rgba[3];                                          \n\
    }                                                           \n\
    float result = ((value * range) + min);                     \n\
    return result;                                              \n\
  }                                                             \n\
  vec2 virtualToPhysicalCoords(vec2 normalizedVirtualCoords, vec2 virtualSize, vec2 physicalSize, vec2 recipPhysicalSize) { \n\
    vec2 virtualCoords = (normalizedVirtualCoords * virtualSize); \n\
    float elementOffset = ((floor(virtualCoords.y) * virtualSize.x) + floor(virtualCoords.x));\n\
    vec2 physicalCoords;                                        \n\
    physicalCoords.x = (mod(elementOffset, physicalSize.x) + 0.5);      \n\
    physicalCoords.y = (floor((elementOffset + 0.5) * recipPhysicalSize.x) + 0.5); \n\
    vec2 normalizedPhysicalCoords = (physicalCoords * recipPhysicalSize); \n\
    return normalizedPhysicalCoords;                            \n\
  }                                                             \n\
  vec2 virtualToPhysicalCoordsFixed(vec2 normalizedVirtualCoords, vec2 virtualSize, vec2 physicalSize, vec2 recipPhysicalSize, float elementsPerPixelScale) { \n\
    vec2 virtualCoords = (normalizedVirtualCoords * virtualSize); \n\
    float elementOffset = ((floor(virtualCoords.y) * virtualSize.x * elementsPerPixelScale) + floor(virtualCoords.x * elementsPerPixelScale));\n\
    vec2 physicalCoords;                                        \n\
    physicalCoords.x = (mod(elementOffset, physicalSize.x) + 0.5);      \n\
    physicalCoords.y = (floor((elementOffset + 0.5) * recipPhysicalSize.x) + 0.5); \n\
    vec2 normalizedPhysicalCoords = (physicalCoords * recipPhysicalSize); \n\
    return normalizedPhysicalCoords;                            \n\
  }                                                             \n\
  vec2 physicalToVirtualCoords(vec2 normalizedPhysicalCoords, vec2 virtualSize, vec2 recipVirtualSize, vec2 physicalSize) { \n\
    vec2 physicalCoords = (normalizedPhysicalCoords * physicalSize); \n\
    float elementOffset = ((floor(physicalCoords.y) * physicalSize.x) + floor(physicalCoords.x));\n\
    vec2 virtualCoords;                                        \n\
    virtualCoords.x = (mod(elementOffset, virtualSize.x) + 0.5);       \n\
    virtualCoords.y = (floor((elementOffset + 0.5) * recipVirtualSize.x) + 0.5); \n\
    vec2 normalizedVirtualCoords = (virtualCoords * recipVirtualSize); \n\
    return normalizedVirtualCoords;                            \n\
  }                                                             \n\
  void main(void) {                                             \n\
    vec2 cPhysicalCoords = (outTexCoord * cRecipPhysicalSize); \n\
    vec2 texCoord = physicalToVirtualCoords(cPhysicalCoords, cVirtualSize, cRecipVirtualSize, cPhysicalSize); \n\
    float i = (texCoord.x * cVirtualSize.x);                    \n\
    float j = (texCoord.y * cVirtualSize.y);                    \n\
    float cValue;                                               \n\
    if (beta != 0.0) {                                          \n\
      cValue = decode32(texture2D(c, cPhysicalCoords));             \n\
    } else {                                                    \n\
      cValue = 0.0;                                             \n\
    }                                                           \n\
    float total = 0.0;                                          \n\
    for (int l = 0; l < 24; l += 1) {                            \n\
      float lCoord = (float(l) + 0.5);                          \n\
      vec2 aInputCoords = vec2(i, lCoord);                      \n\
      vec2 aTransformedCoords = vec2(dot(aInputCoords, aXTransform), dot(aInputCoords, aYTransform)); \n\
      int aComponent = int(mod((floor(aTransformedCoords.x)), 4.0)); \n\
      vec2 aVirtualCoords = (aTransformedCoords * aRecipVirtualSize); \n\
      vec2 aPhysicalCoords = virtualToPhysicalCoordsFixed(aVirtualCoords, aVirtualSize, aPhysicalSize, aRecipPhysicalSize, 0.25); \n\
      float aValue = decode8(texture2D(a, aPhysicalCoords), aComponent, aMin, aRange);    \n\
      vec2 bVirtualCoords = (vec2(lCoord, j) * bRecipVirtualSize); \n\
      vec2 bPhysicalCoords = virtualToPhysicalCoords(bVirtualCoords, bVirtualSize, bPhysicalSize, bRecipPhysicalSize); \n\
      float bValue = decode32(texture2D(b, bPhysicalCoords));   \n\
      total += (aValue * bValue);                               \n\
      if (l >= k) {                                             \n\
        break;                                                  \n\
      }                                                         \n\
    }                                                           \n\
    gl_FragColor = encode32((alpha * total) + (beta * cValue)); \n\
  }                                                             \n\
";

static const char* g_gemmFragmentShader4x = "                   \n\
  varying vec2 outTexCoord;                                     \n\
  uniform sampler2D a;                                          \n\
  uniform vec2 aXTransform;                                     \n\
  uniform vec2 aYTransform;                                     \n\
  uniform vec2 aCoordScale;                                     \n\
  uniform sampler2D b;                                          \n\
  uniform vec2 bCoordScale;                                     \n\
  uniform sampler2D c;                                          \n\
  uniform vec2 cCoordScale;                                     \n\
  uniform float alpha;                                          \n\
  uniform float beta;                                           \n\
  uniform int k;                                                \n\
  void main(void) {                                             \n\
    vec2 texCoord = outTexCoord;                                \n\
    float startI = ((texCoord.x - 0.5) * 4.0) + 0.5;            \n\
    float j = texCoord.y;                                       \n\
    vec2 cCoords = vec2(texCoord.x, texCoord.y) * cCoordScale;  \n\
    vec4 cPixel = texture2D(c, cCoords);                        \n\
    for (int iInc = 0; iInc < 4; iInc += 1) {                   \n\
      float i = startI + float(iInc);                           \n\
      float cValue;                                             \n\
      if (iInc == 0) {                                          \n\
        cValue = cPixel.x;                                      \n\
      } else if (iInc == 1) {                                   \n\
        cValue = cPixel.y;                                      \n\
      } else if (iInc == 2) {                                   \n\
        cValue = cPixel.z;                                      \n\
      } else if (iInc == 3) {                                   \n\
        cValue = cPixel.w;                                      \n\
      }                                                         \n\
      float total;                                              \n\
      if (beta != 0.0) {                                        \n\
        total = (cValue * beta);                                \n\
      } else {                                                  \n\
        total = 0.0;                                            \n\
      }                                                         \n\
      for (int l = 0; l < 24; l += 1) {                          \n\
        float aLCoord = float(l) + 0.5;                         \n\
        vec2 aInputCoords = vec2(i, aLCoord);                   \n\
        vec2 aTransformedCoords = vec2(dot(aInputCoords, aXTransform), dot(aInputCoords, aYTransform)); \n\
        int aXComponent = int(mod(aTransformedCoords.x, 4.0)); \n\
        aTransformedCoords.x = float(int(aTransformedCoords.x / 4.0)) + 0.5;    \n\
        vec2 aCoords = (aTransformedCoords * aCoordScale);        \n\
        vec4 aPixel = texture2D(a, aCoords);                    \n\
        float aValue;                                           \n\
        if (aXComponent == 0) {                                 \n\
          aValue = aPixel.x;                                    \n\
        } else if (aXComponent == 1) {                          \n\
          aValue = aPixel.y;                                    \n\
        } else if (aXComponent == 2) {                          \n\
          aValue = aPixel.z;                                    \n\
        } else {                                                \n\
          aValue = aPixel.w;                                    \n\
        }                                                       \n\
        float bLCoord = float(l / 4) + 0.5;                     \n\
        int bLComponent = int(mod(float(l), 4.0));              \n\
        vec2 bCoords = vec2(bLCoord, j) * bCoordScale;          \n\
        vec4 bPixel = texture2D(b, bCoords);                    \n\
        float bValue;                                           \n\
        if (bLComponent == 0) {                                 \n\
          bValue = bPixel.x;                                    \n\
        } else if (bLComponent == 1) {                          \n\
          bValue = bPixel.y;                                    \n\
        } else if (bLComponent == 2) {                          \n\
          bValue = bPixel.z;                                    \n\
        } else if (bLComponent == 3) {                          \n\
          bValue = bPixel.w;                                    \n\
        }                                                       \n\
        total += (aValue * bValue);                             \n\
        if (l >= k) {                                           \n\
          break;                                                \n\
        }                                                       \n\
      }                                                         \n\
      float result = (alpha * total);                           \n\
      if (iInc == 0) {                                          \n\
        gl_FragColor.r = result;                                \n\
      } else if (iInc == 1) {                                   \n\
        gl_FragColor.g = result;                                \n\
      } else if (iInc == 2) {                                   \n\
        gl_FragColor.b = result;                                \n\
      } else if (iInc == 3) {                                   \n\
        gl_FragColor.a = result;                                \n\
      }                                                         \n\
    }                                                           \n\
  }                                                             \n\
";

static const char* g_gemmUniformNames[] = {
  "modelViewProjectionMatrix",
  "a",
  "aXTransform",
  "aYTransform",
  "aVirtualSize",
  "aRecipVirtualSize",
  "aPhysicalSize",
  "aRecipPhysicalSize",
  "b",
  "bVirtualSize",
  "bRecipVirtualSize",
  "bPhysicalSize",
  "bRecipPhysicalSize",
  "c",
  "cVirtualSize",
  "cRecipVirtualSize",
  "cPhysicalSize",
  "cRecipPhysicalSize",
  "alpha",
  "beta",
  "k",
};
static const int g_gemmUniformCount = (sizeof(g_gemmUniformNames) / sizeof(g_gemmUniformNames[0]));
static GLint g_gemmUniformIds[g_gemmUniformCount];

static const char* g_gemm16BitUniformNames[] = {
  "modelViewProjectionMatrix",
  "a",
  "aXTransform",
  "aYTransform",
  "aVirtualSize",
  "aRecipVirtualSize",
  "aPhysicalSize",
  "aRecipPhysicalSize",
  "aMin",
  "aRange",
  "b",
  "bVirtualSize",
  "bRecipVirtualSize",
  "bPhysicalSize",
  "bRecipPhysicalSize",
  "c",
  "cVirtualSize",
  "cRecipVirtualSize",
  "cPhysicalSize",
  "cRecipPhysicalSize",
  "alpha",
  "beta",
  "k",
};
static const int g_gemm16BitUniformCount = (sizeof(g_gemm16BitUniformNames) / sizeof(g_gemm16BitUniformNames[0]));
static GLint g_gemm16BitUniformIds[g_gemm16BitUniformCount];

static const char* g_gemm8BitUniformNames[] = {
  "modelViewProjectionMatrix",
  "a",
  "aXTransform",
  "aYTransform",
  "aVirtualSize",
  "aRecipVirtualSize",
  "aPhysicalSize",
  "aRecipPhysicalSize",
  "aMin",
  "aRange",
  "b",
  "bVirtualSize",
  "bRecipVirtualSize",
  "bPhysicalSize",
  "bRecipPhysicalSize",
  "c",
  "cVirtualSize",
  "cRecipVirtualSize",
  "cPhysicalSize",
  "cRecipPhysicalSize",
  "alpha",
  "beta",
  "k",
};
static const int g_gemm8BitUniformCount = (sizeof(g_gemm8BitUniformNames) / sizeof(g_gemm8BitUniformNames[0]));
static GLint g_gemm8BitUniformIds[g_gemm8BitUniformCount];

static const char* g_gemm4xUniformNames[] = {
  "modelViewProjectionMatrix",
  "a",
  "aXTransform",
  "aYTransform",
  "aCoordScale",
  "b",
  "bCoordScale",
  "c",
  "cCoordScale",
  "alpha",
  "beta",
  "k",
};
static const int g_gemm4xUniformCount = (sizeof(g_gemm4xUniformNames) / sizeof(g_gemm4xUniformNames[0]));
static GLint g_gemm4xUniformIds[g_gemm4xUniformCount];

// The largest loop to allow in GLSL programs
const int maxLoopSize = 24;

static GLContext* g_glContext = NULL;

void gl_gemm(
  int order,
  int transposeA,
  int transposeB,
  int m,
  int n,
  int inputK,
  jpfloat_t alpha,
  jpfloat_t *a,
  int lda,
  jpfloat_t *b,
  int ldb,
  jpfloat_t inputBeta,
  jpfloat_t* c,
  int ldc) {

fprintf(stderr, "Calling gl_gemm()\n");

  assert((transposeA == JPCblasNoTrans) || (transposeA == JPCblasTrans));
  assert(transposeB == JPCblasNoTrans);
  assert(order == JPCblasColMajor);

  if (g_glContext == NULL) {
    g_glContext = new GLContext();
  }
  GLContext* context = g_glContext;

  GLBuffer* previousCBuffer = NULL;

  const bool use4x = (((m % 4) == 0) && ((inputK % 4) == 0));
  GLProgram* program;
  Dimensions* aFullDims;
  Dimensions* bFullDims;
  Dimensions* cDims;
  if (use4x) {
    fprintf(stderr, "Compiling 4x float shader\n");
    program = new GLProgram(context, g_gemmVertexShader, g_gemmFragmentShader4x, g_gemm4xUniformNames, g_gemm4xUniformIds, g_gemm4xUniformCount);
    if (transposeA == JPCblasTrans) {
      aFullDims = new Dimensions(m, (inputK / 4), 4);
    } else {
      aFullDims = new Dimensions(inputK, (m / 4), 4);
    }
    bFullDims = new Dimensions(n, (inputK / 4), 4);
    cDims = new Dimensions(n, (m / 4), 4);
  } else {
    fprintf(stderr, "Compiling float shader\n");
    program = new GLProgram(context, g_gemmVertexShader, g_gemmFragmentShader, g_gemmUniformNames, g_gemmUniformIds, g_gemmUniformCount);
    if (transposeA == JPCblasTrans) {
      aFullDims = new Dimensions(m, inputK, 1);
    } else {
      aFullDims = new Dimensions(inputK, m, 1);
    }
    bFullDims = new Dimensions(n, inputK, 1);
    cDims = new Dimensions(n, m, 1);
  }
  const bool useVirtualSize = (!use4x);
  Dimensions* cPhysicalDims = physicalFromVirtualSize(cDims, useVirtualSize);

  Buffer* aFullBuffer = new Buffer(*aFullDims, a);
  Buffer* bFullBuffer = new Buffer(*bFullDims, b);

  const int kStepCount = (int)ceilf(inputK / (float)(maxLoopSize));
  for (int kStep = 0; kStep < kStepCount; kStep += 1) {
    int currentK = (inputK - (kStep * maxLoopSize));
    if (currentK > maxLoopSize) {
      currentK = maxLoopSize;
    }
    const int originK = (kStep * maxLoopSize);
    Dimensions* aDims;
    Dimensions* bDims;
    if (use4x) {
      if (transposeA == JPCblasTrans) {
        aDims = new Dimensions(m, (currentK / 4), 4);
      } else {
        aDims = new Dimensions(currentK, (m / 4), 4);
      }
      bDims = new Dimensions(n, (currentK / 4), 4);
    } else {
      if (transposeA == JPCblasTrans) {
        aDims = new Dimensions(m, currentK, 1);
      } else {
        aDims = new Dimensions(currentK, m, 1);
      }
      bDims = new Dimensions(n, currentK, 1);
    }

    Dimensions* aPhysicalDims = physicalFromVirtualSize(aDims, useVirtualSize);
    Dimensions* bPhysicalDims = physicalFromVirtualSize(bDims, useVirtualSize);

    Buffer* aHostBuffer = NULL;
    Buffer* bHostBuffer = NULL;
    if (transposeA == JPCblasTrans) {
      if (use4x) {
        aHostBuffer = extract_subregion(aFullBuffer, Offset(0, (originK / 4), 0), *aDims);
      } else {
        aHostBuffer = extract_subregion(aFullBuffer, Offset(0, originK, 0), *aDims);
      }
    } else {
      aHostBuffer = extract_subregion(aFullBuffer, Offset(originK, 0, 0), *aDims);
    }
    if (use4x) {
      bHostBuffer = extract_subregion(bFullBuffer, Offset(0, (originK / 4), 0), *bDims);
    } else {
      bHostBuffer = extract_subregion(bFullBuffer, Offset(0, originK, 0), *bDims);
    }
    GLBuffer* aBuffer = new GLBuffer(context, *aPhysicalDims, aHostBuffer->_data);
    GLBuffer* bBuffer = new GLBuffer(context, *bPhysicalDims, bHostBuffer->_data);

    jpfloat_t beta;
    if (kStep == 0) {
      if (inputBeta > 0.0f) {
        previousCBuffer = new GLBuffer(context, *cPhysicalDims, c);
      } else {
        previousCBuffer = new GLBuffer(context, *cPhysicalDims);
      }
      beta = inputBeta;
    } else {
      beta = 1.0f;
    }
    GLBuffer* outputCBuffer = new GLBuffer(context, *cPhysicalDims);

    if (transposeA == JPCblasTrans) {
      program->setUniform2f("aXTransform", 0.0f, 1.0f);
      program->setUniform2f("aYTransform", 1.0f, 0.0f);
    } else {
      program->setUniform2f("aXTransform", 1.0f, 0.0f);
      program->setUniform2f("aYTransform", 0.0f, 1.0f);
    }
    if (useVirtualSize) {
      program->setUniform2f("aVirtualSize", (*aDims)[1], (*aDims)[0]);
      program->setUniform2f("aRecipVirtualSize", 1.0f / (*aDims)[1], 1.0f / (*aDims)[0]);
      program->setUniform2f("aPhysicalSize", (*aPhysicalDims)[1], (*aPhysicalDims)[0]);
      program->setUniform2f("aRecipPhysicalSize", 1.0f / (*aPhysicalDims)[1], 1.0f / (*aPhysicalDims)[0]);

      program->setUniform2f("bVirtualSize", (*bDims)[1], (*bDims)[0]);
      program->setUniform2f("bRecipVirtualSize", 1.0f / (*bDims)[1], 1.0f / (*bDims)[0]);
      program->setUniform2f("bPhysicalSize", (*bPhysicalDims)[1], (*bPhysicalDims)[0]);
      program->setUniform2f("bRecipPhysicalSize", 1.0f / (*bPhysicalDims)[1], 1.0f / (*bPhysicalDims)[0]);

      program->setUniform2f("cVirtualSize", (*cDims)[1], (*cDims)[0]);
      program->setUniform2f("cRecipVirtualSize", 1.0f / (*cDims)[1], 1.0f / (*cDims)[0]);
      program->setUniform2f("cPhysicalSize", (*cPhysicalDims)[1], (*cPhysicalDims)[0]);
      program->setUniform2f("cRecipPhysicalSize", 1.0f / (*cPhysicalDims)[1], 1.0f / (*cPhysicalDims)[0]);
    } else {
      program->setUniform2f("aCoordScale", 1.0f / (*aDims)[1], 1.0f / (*aDims)[0]);
      program->setUniform2f("bCoordScale", 1.0f / (*bDims)[1], 1.0f / (*bDims)[0]);
      program->setUniform2f("cCoordScale", 1.0f / (*cDims)[1], 1.0f / (*cDims)[0]);
    }
    program->setUniform1f("alpha", alpha);
    program->setUniform1f("beta", beta);
    program->setUniform1i("k", currentK);
    program->clearInputBuffers();
    program->setInputBuffer("a", aBuffer);
    program->setInputBuffer("b", bBuffer);
    program->setInputBuffer("c", previousCBuffer);

    context->setProgram(program);
    context->setOutputBuffer(outputCBuffer);

    context->runProgram();

    delete previousCBuffer;
    previousCBuffer = outputCBuffer;

    delete bBuffer;
    delete aBuffer;

    if (aHostBuffer != NULL) {
      delete aHostBuffer;
    }
    if (bHostBuffer != NULL) {
      delete bHostBuffer;
    }
    delete bPhysicalDims;
    delete aPhysicalDims;
    delete bDims;
    delete aDims;
  }

  Buffer* output = new Buffer(*cPhysicalDims, c);
  context->copyOutputToHost(output);

  delete aFullDims;
  delete bFullDims;
  delete aFullBuffer;
  delete bFullBuffer;
  delete cDims;
  delete cPhysicalDims;
  delete output; // Doesn't own 'c' data, so this is ok
  delete program;
}

void gl_gemm_fixed(
  int order,
  int transposeA,
  int transposeB,
  int m,
  int n,
  int inputK,
  jpfloat_t alpha,
  void *a,
  jpfloat_t aMin,
  jpfloat_t aMax,
  int aBitsPerElement,
  int lda,
  jpfloat_t *b,
  int ldb,
  jpfloat_t inputBeta,
  jpfloat_t* c,
  int ldc) {

fprintf(stderr, "Calling gl_gemm_fixed()\n");
fprintf(stderr, "m=%d, n=%d, inputK=%d\n", m, n, inputK);

  assert((transposeA == JPCblasNoTrans) || (transposeA == JPCblasTrans));
  assert(transposeB == JPCblasNoTrans);
  assert(order == JPCblasColMajor);

  if (g_glContext == NULL) {
    g_glContext = new GLContext();
  }
  GLContext* context = g_glContext;

  GLBuffer* previousCBuffer = NULL;

  GLProgram* program;
  if (aBitsPerElement == 16) {
    fprintf(stderr, "Compiling 16-bit shader\n");
    program = new GLProgram(context, g_gemmVertexShader, g_gemmFragmentShader16Bit, g_gemm16BitUniformNames, g_gemm16BitUniformIds, g_gemm16BitUniformCount);
  } else if (aBitsPerElement == 8) {
    fprintf(stderr, "Compiling 8-bit shader\n");
    program = new GLProgram(context, g_gemmVertexShader, g_gemmFragmentShader8Bit, g_gemm8BitUniformNames, g_gemm8BitUniformIds, g_gemm8BitUniformCount);
  } else {
    assert(false); // Should never get here
  }

  Dimensions* aFullDims;
  if (transposeA == JPCblasTrans) {
    aFullDims = new Dimensions(m, inputK, 1);
  } else {
    aFullDims = new Dimensions(inputK, m, 1);
  }
  Dimensions* bFullDims = new Dimensions(n, inputK, 1);
  Dimensions* cDims = new Dimensions(n, m, 1);

  const bool useVirtualSize = true;
  Dimensions* cPhysicalDims = physicalFromVirtualSize(cDims, useVirtualSize);

  Buffer* aFullBuffer = new Buffer(*aFullDims, a, aMin, aMax, aBitsPerElement);
  Buffer* bFullBuffer = new Buffer(*bFullDims, b);

  const jpfloat_t aSpread = (aMax - aMin);
  const jpfloat_t fullBitRange = (1 << aBitsPerElement);
  const jpfloat_t oneOffBitRange = (fullBitRange - 1);

  const jpfloat_t aRange = ((aSpread * oneOffBitRange) / fullBitRange);

  const int kStepCount = (int)ceilf(inputK / (float)(maxLoopSize));
  for (int kStep = 0; kStep < kStepCount; kStep += 1) {
    int currentK = (inputK - (kStep * maxLoopSize));
    if (currentK > maxLoopSize) {
      currentK = maxLoopSize;
    }
    const int originK = (kStep * maxLoopSize);
    Dimensions* aDims;
    if (transposeA == JPCblasTrans) {
      aDims = new Dimensions(m, currentK, 1);
    } else {
      aDims = new Dimensions(currentK, m, 1);
    }
    Dimensions* bDims = new Dimensions(n, currentK, 1);

    fprintf(stderr, "aDims=%s, bDims=%s\n", aDims->debugString(), bDims->debugString());
    Dimensions* aPhysicalDims = physicalFromVirtualSize(aDims, useVirtualSize, aBitsPerElement);
    Dimensions* bPhysicalDims = physicalFromVirtualSize(bDims, useVirtualSize);

    Buffer* aHostBuffer = NULL;
    if (transposeA == JPCblasTrans) {
      aHostBuffer = extract_subregion(aFullBuffer, Offset(0, originK, 0), *aDims);
    } else {
      aHostBuffer = extract_subregion(aFullBuffer, Offset(originK, 0, 0), *aDims);
    }
    Buffer* bHostBuffer = extract_subregion(bFullBuffer, Offset(0, originK, 0), *bDims);
    GLBuffer* aBuffer = new GLBuffer(context, *aPhysicalDims, aHostBuffer->_quantizedData, aMin, aMax, aBitsPerElement);
    GLBuffer* bBuffer = new GLBuffer(context, *bPhysicalDims, bHostBuffer->_data);

    jpfloat_t beta;
    if (kStep == 0) {
      if (inputBeta > 0.0f) {
        previousCBuffer = new GLBuffer(context, *cPhysicalDims, c);
      } else {
        previousCBuffer = new GLBuffer(context, *cPhysicalDims);
      }
      beta = inputBeta;
    } else {
      beta = 1.0f;
    }
    GLBuffer* outputCBuffer = new GLBuffer(context, *cPhysicalDims);

    if (transposeA == JPCblasTrans) {
      program->setUniform2f("aXTransform", 0.0f, 1.0f);
      program->setUniform2f("aYTransform", 1.0f, 0.0f);
    } else {
      program->setUniform2f("aXTransform", 1.0f, 0.0f);
      program->setUniform2f("aYTransform", 0.0f, 1.0f);
    }
    program->setUniform2f("aVirtualSize", (*aDims)[1], (*aDims)[0]);
    program->setUniform2f("aRecipVirtualSize", 1.0f / (*aDims)[1], 1.0f / (*aDims)[0]);
    program->setUniform2f("aPhysicalSize", (*aPhysicalDims)[1], (*aPhysicalDims)[0]);
    program->setUniform2f("aRecipPhysicalSize", 1.0f / (*aPhysicalDims)[1], 1.0f / (*aPhysicalDims)[0]);
    program->setUniform1f("aMin", aMin);
    program->setUniform1f("aRange", aRange);

    program->setUniform2f("bVirtualSize", (*bDims)[1], (*bDims)[0]);
    program->setUniform2f("bRecipVirtualSize", 1.0f / (*bDims)[1], 1.0f / (*bDims)[0]);
    program->setUniform2f("bPhysicalSize", (*bPhysicalDims)[1], (*bPhysicalDims)[0]);
    program->setUniform2f("bRecipPhysicalSize", 1.0f / (*bPhysicalDims)[1], 1.0f / (*bPhysicalDims)[0]);

    program->setUniform2f("cVirtualSize", (*cDims)[1], (*cDims)[0]);
    program->setUniform2f("cRecipVirtualSize", 1.0f / (*cDims)[1], 1.0f / (*cDims)[0]);
    program->setUniform2f("cPhysicalSize", (*cPhysicalDims)[1], (*cPhysicalDims)[0]);
    program->setUniform2f("cRecipPhysicalSize", 1.0f / (*cPhysicalDims)[1], 1.0f / (*cPhysicalDims)[0]);

    program->setUniform1f("alpha", alpha);
    program->setUniform1f("beta", beta);
    program->setUniform1i("k", currentK);

    program->clearInputBuffers();
    program->setInputBuffer("a", aBuffer);
    program->setInputBuffer("b", bBuffer);
    program->setInputBuffer("c", previousCBuffer);

    context->setProgram(program);
    context->setOutputBuffer(outputCBuffer);

    context->runProgram();

    delete previousCBuffer;
    previousCBuffer = outputCBuffer;

    delete bBuffer;
    delete aBuffer;

    if (aHostBuffer != NULL) {
      delete aHostBuffer;
    }
    if (bHostBuffer != NULL) {
      delete bHostBuffer;
    }
    delete bPhysicalDims;
    delete aPhysicalDims;
    delete bDims;
    delete aDims;
  }

  Buffer* output = new Buffer(*cPhysicalDims, c);
  context->copyOutputToHost(output);

  delete aFullDims;
  delete bFullDims;
  delete aFullBuffer;
  delete bFullBuffer;
  delete cDims;
  delete cPhysicalDims;
  delete output; // Doesn't own 'c' data, so this is ok
  delete program;
}

Dimensions* physicalFromVirtualSize(Dimensions* virtualSize, bool doResize, int bitsPerElement) {
  assert(virtualSize->_length == 3);

  const int elementsPerPixel = (32 / bitsPerElement);

  const int virtualWidth = virtualSize->_dims[1];
  const int virtualHeight = virtualSize->_dims[0];
  const int virtualChannels = virtualSize->_dims[2];

#if DEBUG
  if ((virtualWidth % elementsPerPixel) != 0) {
    fprintf(stderr, "Bad virtualWidth/elementsPerPixel ratio: %d/%d\n", virtualWidth, elementsPerPixel);
  }
#endif // DEBUG
  assert((virtualWidth % elementsPerPixel) == 0);

  const int virtualPixelCount = ((virtualWidth / elementsPerPixel) * virtualHeight);

  int physicalWidth = (virtualWidth / elementsPerPixel);
  int physicalHeight = virtualHeight;

  float distanceFromSquareness = fabsf(physicalWidth - physicalHeight);

  const int maxFactor = (doResize) ? 20 : 0;
  for (int factor = 2; factor < maxFactor; factor += 1) {
    int candidateWidth;
    int candidateHeight;
    if (virtualWidth > virtualHeight) {
      candidateWidth = (virtualWidth / factor);
      candidateHeight = (virtualHeight * factor);
    } else {
      candidateWidth = (virtualWidth * factor);
      candidateHeight = (virtualHeight / factor);
    }
    if ((candidateWidth % elementsPerPixel) != 0) {
      continue;
    }
    candidateWidth = (candidateWidth / elementsPerPixel);
    const int candidatePixelCount = (candidateWidth * candidateHeight);
    const float candidateDistanceFromSquareness = fabsf(candidateWidth - candidateHeight);
    if ((candidatePixelCount == virtualPixelCount) && (candidateDistanceFromSquareness < distanceFromSquareness)) {
      physicalWidth = candidateWidth;
      physicalHeight = candidateHeight;
      distanceFromSquareness = candidateDistanceFromSquareness;
    }
  }

  Dimensions* result = new Dimensions(physicalHeight, physicalWidth, virtualChannels);
  return result;
}

void test_gl_gemm() {
#if 0
  const int inputChannels = 363;
  const int inputHeight = 3000;
  const int outputChannels = 96;
  Buffer* input = new Buffer(Dimensions(inputHeight, inputChannels));
  Buffer* weights = new Buffer(Dimensions(outputChannels, inputChannels));
  const bool areWeightsTransposed = true;

  const Dimensions inputDims = input->_dims;
  // We're expecting (# of images, # of values)
  assert(inputDims._length == 2);

  const int imageCount = inputDims[0];
  const int inputValuesCount = inputDims[1];

  const Dimensions weightsDims = weights->_dims;
  // We're expecting (# of values in input, # of output channels)
  assert(inputDims._length == 2);
  assert(inputDims._length == 2);
  int inputValuesIndex;
  int outputChannelsIndex;
  if (areWeightsTransposed) {
    inputValuesIndex = 1;
    outputChannelsIndex = 0;
  } else {
    inputValuesIndex = 0;
    outputChannelsIndex = 1;
  }
  assert(weightsDims[inputValuesIndex] == inputValuesCount);
//  const int outputChannels = weightsDims[outputChannelsIndex];

  const Dimensions outputDims(imageCount, outputChannels);
  Buffer* outputCPU = new Buffer(outputDims);
  outputCPU->setName("outputCPU");
  Buffer* outputGPU = new Buffer(outputDims);
  outputGPU->setName("outputGPU");

  input->populateWithRandomValues(0, 1);
  weights->populateWithRandomValues(0, 1);

  const int order = JPCblasColMajor;
  const int transposeA = JPCblasTrans;
  const int transposeB = JPCblasNoTrans;

  const int m = outputChannels;
  const int n = input->_dims[0];
  const int k = input->_dims[1];
  const float alpha = 1.0f;
  const int lda = (transposeA == JPCblasNoTrans) ? m : k;
  const int ldb = k;
  const int ldc = m;
  const jpfloat_t beta = 0.0f;

//  naive_cblas_sgemm(
//    order,
//    transposeA,
//    transposeB,
//    m,
//    n,
//    k,
//    alpha,
//    weights->_data,
//    lda,
//    input->_data,
//    ldb,
//    beta,
//    outputCPU->_data,
//    ldc
//  );
//
//  eigen_cblas_sgemm(
//    order,
//    transposeA,
//    transposeB,
//    m,
//    n,
//    k,
//    alpha,
//    weights->_data,
//    lda,
//    input->_data,
//    ldb,
//    beta,
//    outputGPU->_data,
//    ldc
//  );

//  outputCPU->printContents();
//  outputGPU->printContents();
//  assert(buffer_are_all_close(outputCPU, outputGPU));

  const float weightsMin = 0.0f;
  const float weightsMax = 1.0f;
  const int weightsBitsPerElement = 16;

  Buffer* weightsFixed = new Buffer(Dimensions(outputChannels, inputChannels), weightsMin, weightsMax, weightsBitsPerElement);
  Buffer* outputFixedCPU = new Buffer(outputDims);
  outputFixedCPU->setName("outputFixedCPU");
  Buffer* outputFixedGPU = new Buffer(outputDims);
  outputFixedGPU->setName("outputFixedGPU");

  weightsFixed->populateWithRandomValues(0, 1);

  naive_cblas_sgemm_fixed(
    order,
    transposeA,
    transposeB,
    m,
    n,
    k,
    alpha,
    weightsFixed->_quantizedData,
    weightsMin,
    weightsMax,
    weightsBitsPerElement,
    lda,
    input->_data,
    ldb,
    beta,
    outputFixedCPU->_data,
    ldc
  );

  cblas_sgemm_fixed(
    order,
    transposeA,
    transposeB,
    m,
    n,
    k,
    alpha,
    weightsFixed->_quantizedData,
    weightsMin,
    weightsMax,
    weightsBitsPerElement,
    lda,
    input->_data,
    ldb,
    beta,
    outputFixedGPU->_data,
    ldc
  );

//  outputFixedCPU->printContents();
//  outputFixedGPU->printContents();
  assert(buffer_are_all_close(outputFixedCPU, outputFixedGPU));
#endif
}

#endif // USE_OPENGL
