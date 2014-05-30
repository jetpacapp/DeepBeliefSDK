#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <sys/time.h>
#include <assert.h>

#include "mailbox.h"
#include "buffer.h"
#include "matrix_ops.h"
#include "qpu_gemm.h"

#define NUM_QPUS        (8)
#define NUM_MESSAGE_VALUES (2)

#define DO_LOG_OPERATIONS

extern uint32_t g_gemm_8bitCode[];
extern size_t g_gemm_8bitCodeByteCount;

extern uint32_t g_gemm_16bitCode[];
extern size_t g_gemm_16bitCodeByteCount;

extern uint32_t g_gemm_floatCode[];
extern size_t g_gemm_floatCodeByteCount;

uint32_t floatAsUInt32(float input) {
  return *((uint32_t*)&input);
}

void qpu_cblas_sgemm(
  int order,
  int transposeA,
  int transposeB,
  int m,
  int n,
  int k,
  float alpha,
  uint32_t a,
  int lda,
  uint32_t b,
  int ldb,
  float beta,
  uint32_t c,
  int ldc) {

  qpu_cblas_sgemm_fixed(
    order,
    transposeA,
    transposeB,
    m,
    n,
    k,
    alpha,
    a,
    0.0f,
    1.0f,
    32,
    lda,
    b,
    ldb,
    beta,
    c,
    ldc);
}

void qpu_cblas_sgemm_fixed(
  int order,
  int transposeA,
  int transposeB,
  int m,
  int n,
  int k,
  float alpha,
  uint32_t a,
  float aMin,
  float aMax,
  int aBitsPerElement,
  int lda,
  uint32_t b,
  int ldb,
  float beta,
  uint32_t c,
  int ldc) {

#ifdef DO_LOG_OPERATIONS
  fprintf(stderr, "qpu_cblas_sgemm_fixed(\n  m=%d,\n  n=%d,\n  k=%d,\n  alpha=%f,\n  aMin=%f,\n  aMax=%f\n,  aBitsPerElement=%d,\n  a=%p,\n  lda=%d,\n  b=%p,\n  ldb=%d,\n  beta=%f,\n  c=%p,\n  ldc=%d)\n",
    m,
    n,
    k,
    alpha,
    aMin,
    aMax,
    aBitsPerElement,
    a,
    lda,
    b,
    ldb,
    beta,
    c,
    ldc);
#endif // DO_LOG_OPERATIONS

  assert(transposeA == JPCblasTrans);
  assert(transposeB == JPCblasNoTrans);
  assert(order == JPCblasColMajor);
  assert((aBitsPerElement == 32) || (aBitsPerElement == 16) || (aBitsPerElement ==8));

  struct timeval start;
  gettimeofday(&start, NULL);

  uint32_t* qpuCode = NULL;
  size_t qpuCodeByteCount = 0;
  if (aBitsPerElement == 32) {
    qpuCode = g_gemm_floatCode;
    qpuCodeByteCount = g_gemm_floatCodeByteCount;
  } else if (aBitsPerElement == 16) {
    qpuCode = g_gemm_16bitCode;
    qpuCodeByteCount = g_gemm_16bitCodeByteCount;
  } else if (aBitsPerElement == 8) {
    qpuCode = g_gemm_8bitCode;
    qpuCodeByteCount = g_gemm_8bitCodeByteCount;
  } else {
    assert(false); // Should never get here
    qpuCode = NULL;
  }

  float aRange;
  if (aBitsPerElement == 32) {
    aRange = 1.0f;
  } else if (aBitsPerElement == 16) {
    aRange = ((aMax - aMin) / (1 << aBitsPerElement));
  } else {
    const float rangeCorrection = (((1 << aBitsPerElement) - 1.0f) / (1 << aBitsPerElement));
    aRange = ((aMax - aMin) * rangeCorrection);
  }

  if (qpu_enable(1)) {
      fprintf(stderr, "QPU enable failed.\n");
      return;
  }
  printf("QPU enabled.\n");

  const size_t messageByteCount = (NUM_QPUS * NUM_MESSAGE_VALUES * sizeof(uint32_t));

  const int uniformCount = 15;
  const size_t uniformByteCount = (NUM_QPUS * uniformCount * sizeof(uint32_t));

  const int debugCount = 16;
  const size_t debugByteCount = (NUM_QPUS * debugCount * sizeof(float));

  const size_t totalByteCount = (
    messageByteCount +
    qpuCodeByteCount +
    uniformByteCount +
    debugByteCount);

  uint32_t gpuMemoryHandle = mem_alloc(totalByteCount, 4096, GPU_MEM_FLG);
  if (!gpuMemoryHandle) {
    fprintf(stderr, "Unable to allocate %d bytes of GPU memory", totalByteCount);
    return;
  }
  uint32_t gpuMemoryBase = mem_lock(gpuMemoryHandle);
  char* armMemoryBase = (char*)(mapmem(gpuMemoryBase + GPU_MEM_MAP, totalByteCount));

  const size_t messageOffset = 0;
  const size_t codeOffset = (messageOffset + messageByteCount);
  const size_t uniformOffset = (codeOffset + qpuCodeByteCount);
  const size_t debugOffset = (uniformOffset + uniformByteCount);

  char* messageInputArm = (armMemoryBase + messageOffset);
  char* codeInputArm = (armMemoryBase + codeOffset);
  char* uniformInputsArm = (armMemoryBase + uniformOffset);
  char* debugBaseArm = (armMemoryBase + debugOffset);

  memcpy(codeInputArm, qpuCode, qpuCodeByteCount);

  const uint32_t messageInputGpu = (gpuMemoryBase + messageOffset);
  const uint32_t codeInputGpu = (gpuMemoryBase + codeOffset);
  const uint32_t uniformInputsGpu = (gpuMemoryBase + uniformOffset);
  const uint32_t aInputGpu = a;
  const uint32_t bInputGpu = b;
  const uint32_t cInputGpu = c;
  const uint32_t debugBaseGpu = (gpuMemoryBase + debugOffset);

  for (int i=0; i < NUM_QPUS; i++) {

    const size_t currentDebugOffset = (i * debugCount * sizeof(uint32_t));
    uint32_t* currentDebugArm = (uint32_t*)(debugBaseArm + currentDebugOffset);
    for (int index = 0; index < debugCount; index += 1) {
      currentDebugArm[index] = 0xdeadbeef;
    }
    const uint32_t currentDebugGpu = (debugBaseGpu + currentDebugOffset);

    const size_t currentUniformOffset = (i * uniformCount * sizeof(uint32_t));
    uint32_t* currentUniformsArm = (uint32_t*)(uniformInputsArm + currentUniformOffset);
    currentUniformsArm[0] = m;
    currentUniformsArm[1] = n;
    currentUniformsArm[2] = k;
    currentUniformsArm[3] = floatAsUInt32(alpha);
    currentUniformsArm[4] = aInputGpu;
    currentUniformsArm[5] = floatAsUInt32(aMin);
    currentUniformsArm[6] = floatAsUInt32(aRange);
    currentUniformsArm[7] = lda;
    currentUniformsArm[8] = bInputGpu;
    currentUniformsArm[9] = ldb;
    currentUniformsArm[10] = floatAsUInt32(beta);
    currentUniformsArm[11] = cInputGpu;
    currentUniformsArm[12] = ldc;
    currentUniformsArm[13] = currentDebugGpu;
    currentUniformsArm[14] = i;

    const uint32_t uniformsGpu = (uniformInputsGpu + currentUniformOffset);

    const size_t currentMessageOffset = (i * NUM_MESSAGE_VALUES * sizeof(uint32_t));
    uint32_t* messageArm = (uint32_t*)(messageInputArm + currentMessageOffset);
    messageArm[0] = uniformsGpu;
    messageArm[1] = codeInputGpu;
  }

  unsigned ret = execute_qpu(NUM_QPUS, messageInputGpu, 1, 10000);

//  for (int i=0; i < NUM_QPUS; i++) {
//    const size_t currentDebugOffset = (i * debugCount * sizeof(uint32_t));
//    uint32_t* currentDebugArm = (uint32_t*)(debugBaseArm + currentDebugOffset);
//    for (int index = 0; index < debugCount; index += 1) {
//      fprintf(stderr, "%d:%d=%f (0x%08x, %d)\n", i, index,
//        *(float*)(&currentDebugArm[index]),
//        currentDebugArm[index],
//        currentDebugArm[index]);
//    }
//  }

  unmapmem(armMemoryBase, totalByteCount);
  mem_unlock(gpuMemoryHandle);
  mem_free(gpuMemoryHandle);
  qpu_enable(0);

  struct timeval end;
  gettimeofday(&end, NULL);
  long seconds  = end.tv_sec  - start.tv_sec;
  long useconds = end.tv_usec - start.tv_usec;
  long duration = ((seconds) * 1000 + useconds/1000.0) + 0.5;
  fprintf(stderr, "Took %ldms\n", duration);

}
