//
//  glgemm.cpp
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#include "glgemm.h"

#include <assert.h>

#include "glcontext.h"
#include "glbuffer.h"
#include "glprogram.h"
#include "dimensions.h"

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
  uniform float alpha;                                          \n\
  uniform int k;                                                \n\
  void main(void) {                                             \n\
    vec2 texCoord = outTexCoord;                                \n\
    float i = texCoord.x;                                       \n\
    float j = texCoord.y;                                       \n\
    float total = 0.0; int foo = k;                                         \n\
    for (int l = 0; l < k; l += 1) {                            \n\
      float lCoord = (float(l) + 0.5);                          \n\
      vec2 aCoords = vec2(i, lCoord) * aCoordScale;             \n\
      float aValue = texture2D(a, aCoords).r;                   \n\
      vec2 bCoords = vec2(lCoord, j) * bCoordScale;             \n\
      float bValue = texture2D(b, bCoords).r;                   \n\
      total += (aValue * bValue);                               \n\
    }                                                           \n\
    gl_FragColor.r = (alpha * total);                           \n\
  }                                                             \n\
";

static const char* g_gemmUniformNames[] = {
  "a",
  "aCoordScale",
  "b",
  "bCoordScale",
  "alpha",
  "k",
};
static const int g_gemmUniformCount = (sizeof(g_gemmUniformNames) / sizeof(g_gemmUniformNames[0]));
static GLint g_gemmUniformIds[g_gemmUniformCount];

void gl_gemm(
  int m,
  int n,
  int k,
  jpfloat_t alpha,
  jpfloat_t *a,
  int lda,
  jpfloat_t *b,
  int ldb,
  jpfloat_t beta,
  jpfloat_t* c,
  int ldc) {

  assert(beta == 0.0f);

  GLContext* context = new GLContext();

  const Dimensions aDims(k, m, 1);
  GLBuffer* aBuffer = new GLBuffer(context, aDims, a);

  const Dimensions bDims(n, k, 1);
  GLBuffer* bBuffer = new GLBuffer(context, bDims, b);

  const Dimensions cDims(n, m, 1);
  GLBuffer* cBuffer = new GLBuffer(context, cDims, c);

  GLProgram* program = new GLProgram(context, g_gemmVertexShader, g_gemmFragmentShader, g_gemmUniformNames, g_gemmUniformIds, g_gemmUniformCount);
  program->setUniform2f("aCoordScale", 1.0f / aDims[1], 1.0f / aDims[0]);
  program->setUniform2f("bCoordScale", 1.0f / bDims[1], 1.0f / bDims[0]);
  program->setUniform1f("alpha", alpha);
  program->setUniform1i("k", k);
  program->setInputBuffer("a", aBuffer);
  program->setInputBuffer("b", bBuffer);

  context->setProgram(program);
  context->setOutputBuffer(cBuffer);

  context->runProgram();

  context->copyOutputToHost();

  delete program;

  delete cBuffer;
  delete bBuffer;
  delete aBuffer;

  delete context;
}
