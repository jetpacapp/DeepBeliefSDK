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

static const char* g_gemmVertexShader = "                       \n\
  attribute vec2 aVertexPosition;                               \n\
  attribute vec2 aTexCoord;                                     \n\
  varying vec2 outTexCoord;                                     \n\
                                                                \n\
  void main(void) {                                             \n\
    gl_Position = gl_ModelViewProjectionMatrix * vec4(aVertexPosition, 0.0, 1.0); \n\
    outTexCoord = aTexCoord;                                    \n\
  }                                                             \n\
";

static const char* g_gemmFragmentShader = "                     \n\
  varying vec2 outTexCoord;                                     \n\
  uniform sampler2D a;                                          \n\
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
    float i = texCoord.x;                                       \n\
    float j = texCoord.y;                                       \n\
    vec2 cCoords = vec2(i, j) * cCoordScale;                    \n\
    float cValue;                                               \n\
    if (beta != 0.0) {                                          \n\
      cValue = texture2D(c, cCoords).r;                         \n\
    } else {                                                    \n\
      cValue = 0.0;                                             \n\
    }                                                           \n\
    float total = 0.0;                                          \n\
    for (int l = 0; l < k; l += 1) {                            \n\
      float lCoord = (float(l) + 0.5);                          \n\
      vec2 aCoords = vec2(i, lCoord) * aCoordScale;             \n\
      float aValue = texture2D(a, aCoords).r;                   \n\
      vec2 bCoords = vec2(lCoord, j) * bCoordScale;             \n\
      float bValue = texture2D(b, bCoords).r;                   \n\
      total += (aValue * bValue);                               \n\
    }                                                           \n\
    gl_FragColor.r = (alpha * total) + (beta * cValue);         \n\
  }                                                             \n\
";

static const char* g_gemmFragmentShader4x = "                   \n\
  varying vec2 outTexCoord;                                     \n\
  uniform sampler2D a;                                          \n\
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
      float iCoord = float(int(i) / 4) + 0.5;                   \n\
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
      for (int l = 0; l < k; l += 1) {                          \n\
        float aLCoord = float(l) + 0.5;                         \n\
        vec2 aCoords = vec2(iCoord, aLCoord) * aCoordScale;     \n\
        vec4 aPixel = texture2D(a, aCoords);                    \n\
        float aValue;                                           \n\
        if (iInc == 0) {                                        \n\
          aValue = aPixel.x;                                    \n\
        } else if (iInc == 1) {                                 \n\
          aValue = aPixel.y;                                    \n\
        } else if (iInc == 2) {                                 \n\
          aValue = aPixel.z;                                    \n\
        } else if (iInc == 3) {                                 \n\
          aValue = aPixel.w;                                    \n\
        }                                                       \n\
        float bLCoord = float(l / 4) + 0.5;                     \n\
        int bLComponent = int(mod(float(l), 4.0));                       \n\
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
  "a",
  "aCoordScale",
  "b",
  "bCoordScale",
  "c",
  "cCoordScale",
  "alpha",
  "beta",
  "k",
};
static const int g_gemmUniformCount = (sizeof(g_gemmUniformNames) / sizeof(g_gemmUniformNames[0]));
static GLint g_gemmUniformIds[g_gemmUniformCount];

// Somewhat arbitrary, but covers 85% of cards according to
// http://feedback.wildfiregames.com/report/opengl/feature/GL_MAX_TEXTURE_SIZE
const int maxTextureSize = 4096;

void gl_gemm(
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

  GLContext* context = new GLContext();

  GLBuffer* previousCBuffer = NULL;

  const bool use4x = (((m % 4) == 0) && ((inputK % 4) == 0));
  GLProgram* program;
  Dimensions* aFullDims;
  Dimensions* bFullDims;
  Dimensions* cDims;
  if (use4x) {
    program = new GLProgram(context, g_gemmVertexShader, g_gemmFragmentShader4x, g_gemmUniformNames, g_gemmUniformIds, g_gemmUniformCount);
    aFullDims = new Dimensions(inputK, (m / 4), 4);
    bFullDims = new Dimensions(n, (inputK / 4), 4);
    cDims = new Dimensions(n, (m / 4), 4);
  } else {
    program = new GLProgram(context, g_gemmVertexShader, g_gemmFragmentShader, g_gemmUniformNames, g_gemmUniformIds, g_gemmUniformCount);
    aFullDims = new Dimensions(inputK, m, 1);
    bFullDims = new Dimensions(n, inputK, 1);
    cDims = new Dimensions(n, m, 1);
  }

  Buffer* aFullBuffer = new Buffer(*aFullDims, a);
  Buffer* bFullBuffer = new Buffer(*bFullDims, b);

  const int kStepCount = (int)ceilf(inputK / (float)(maxTextureSize));
  for (int kStep = 0; kStep < kStepCount; kStep += 1) {
    int currentK = (inputK - (kStep * maxTextureSize));
    if (currentK > maxTextureSize) {
      currentK = maxTextureSize;
    }
    const int originK = (kStep * maxTextureSize);
    Dimensions* aDims;
    Dimensions* bDims;
    if (use4x) {
      aDims = new Dimensions(currentK, (m / 4), 4);
      bDims = new Dimensions(n, (currentK / 4), 4);
    } else {
      aDims = new Dimensions(currentK, m, 1);
      bDims = new Dimensions(n, currentK, 1);
    }

    fprintf(stderr, "aDims=%s\n", aDims->debugString());
    fprintf(stderr, "bDims=%s\n", bDims->debugString());
    fprintf(stderr, "cDims=%s\n", cDims->debugString());

    GLBuffer* aBuffer;
    GLBuffer* bBuffer;
    Buffer* aHostBuffer = NULL;
    Buffer* bHostBuffer = NULL;
    if (kStepCount == 0) {
      aBuffer = new GLBuffer(context, *aDims, a);
      bBuffer = new GLBuffer(context, *bDims, b);
    } else {
      aHostBuffer = extract_subregion(aFullBuffer, Offset(originK, 0, 0), *aDims);
      if (use4x) {
        bHostBuffer = extract_subregion(bFullBuffer, Offset(0, (originK / 4), 0), *bDims);
      } else {
        bHostBuffer = extract_subregion(bFullBuffer, Offset(0, originK, 0), *bDims);
      }
      aBuffer = new GLBuffer(context, *aDims, aHostBuffer->_data);
      bBuffer = new GLBuffer(context, *bDims, bHostBuffer->_data);
    }

    jpfloat_t beta;
    if (kStep == 0) {
      if (inputBeta > 0.0f) {
        previousCBuffer = new GLBuffer(context, *cDims, c);
      } else {
        previousCBuffer = new GLBuffer(context, *cDims);
      }
      beta = inputBeta;
    } else {
      beta = 1.0f;
    }
    GLBuffer* outputCBuffer = new GLBuffer(context, *cDims);

    program->setUniform2f("aCoordScale", 1.0f / (*aDims)[1], 1.0f / (*aDims)[0]);
    program->setUniform2f("bCoordScale", 1.0f / (*bDims)[1], 1.0f / (*bDims)[0]);
    program->setUniform2f("cCoordScale", 1.0f / (*cDims)[1], 1.0f / (*cDims)[0]);
    program->setUniform1f("alpha", alpha);
    program->setUniform1f("beta", beta);
    program->setUniform1i("k", currentK);
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
    delete bDims;
    delete aDims;
  }

  Buffer* output = new Buffer(*cDims, c);
  context->copyOutputToHost(output);

  delete aFullDims;
  delete bFullDims;
  delete aFullBuffer;
  delete bFullBuffer;
  delete cDims;
  delete output; // Doesn't own 'c' data, so this is ok
  delete program;
  delete context;
}

#endif // USE_OPENGL