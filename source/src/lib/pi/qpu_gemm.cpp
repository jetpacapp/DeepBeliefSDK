#ifdef USE_QPU_GEMM
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

#define NUM_QPUS        (12)
#define NUM_MESSAGE_VALUES (2)



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
  fprintf(stderr, "qpu_cblas_sgemm_fixed(\n  m=%d,\n  n=%d,\n  k=%d,\n  alpha=%f,\n  aMin=%f,\n  aMax=%f,\n  aBitsPerElement=%d,\n  a=%p,\n  lda=%d,\n  b=%p,\n  ldb=%d,\n  beta=%f,\n  c=%p,\n  ldc=%d)\n",
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

#ifdef DO_LOG_OPERATIONS
  struct timeval start;
  gettimeofday(&start, NULL);
#endif // DO_LOG_OPERATIONS

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

#ifdef DO_LOG_OPERATIONS
  for (int i=0; i < NUM_QPUS; i++) {
    const size_t currentDebugOffset = (i * debugCount * sizeof(uint32_t));
    uint32_t* currentDebugArm = (uint32_t*)(debugBaseArm + currentDebugOffset);
    for (int index = 0; index < debugCount; index += 1) {
      fprintf(stderr, "%d:%d=%f (0x%08x, %d)\n", i, index,
        *(float*)(&currentDebugArm[index]),
        currentDebugArm[index],
        currentDebugArm[index]);
    }
  }
#endif

  unmapmem(armMemoryBase, totalByteCount);
  mem_unlock(gpuMemoryHandle);
  mem_free(gpuMemoryHandle);
  qpu_enable(0);

#ifdef DO_LOG_OPERATIONS
  struct timeval end;
  gettimeofday(&end, NULL);
  long seconds  = end.tv_sec  - start.tv_sec;
  long useconds = end.tv_usec - start.tv_usec;
  long duration = ((seconds) * 1000 + useconds/1000.0) + 0.5;
  fprintf(stderr, "Took %ldms\n", duration);
#endif // DO_LOG_OPERATIONS

}

void test_qpu_gemm() {
//  const int inputChannels = (9216/1);
//  const int inputHeight = 1;
//  const int outputChannels = 4096;
//  const int inputChannels = 147;
//  const int inputHeight = 12321;
//  const int outputChannels = 96;
//  const int inputChannels = 729;
//  const int inputHeight = 1200;
//  const int outputChannels = 128;
//  const int inputChannels = 9216;
//  const int inputHeight = 1;
//  const int outputChannels = 4096;
//  const int inputChannels = 363;
//  const int inputHeight = 3025;
//  const int outputChannels = 96;
  const int inputChannels = 33;//2304;
  const int inputHeight = 169;
  const int outputChannels = 384;

  Buffer* input = new Buffer(Dimensions(inputHeight, inputChannels));
  input->setName("input");
  const bool areWeightsTransposed = true;

  const Dimensions inputDims = input->_dims;
  // We're expecting (# of images, # of values)
  assert(inputDims._length == 2);

  const int imageCount = inputDims[0];
  const int inputValuesCount = inputDims[1];

  const Dimensions weightsDims(outputChannels, inputChannels);
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

  const Dimensions outputDims(imageCount, outputChannels);
  Buffer* outputCPU = new Buffer(outputDims);
  outputCPU->setName("outputCPU");
  Buffer* outputGPU = new Buffer(outputDims);
  outputGPU->setName("outputGPU");

  input->populateWithRandomValues(0, 1);

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
  const float beta = 0.0f;

  const int weightsBitsPerElement = 8;

  if (weightsBitsPerElement == 32) {

    Buffer* weights = new Buffer(weightsDims);
    weights->setName("weights");
    weights->populateWithRandomValues(0, 1);

    naive_cblas_sgemm(
      order,
      transposeA,
      transposeB,
      m,
      n,
      k,
      alpha,
      weights->_data,
      lda,
      input->_data,
      ldb,
      beta,
      outputCPU->_data,
      ldc
    );

    qpu_cblas_sgemm(
      order,
      transposeA,
      transposeB,
      m,
      n,
      k,
      alpha,
      weights->_gpuMemoryBase,
      lda,
      input->_gpuMemoryBase,
      ldb,
      beta,
      outputGPU->_gpuMemoryBase,
      ldc
    );

    delete weights;

  } else {

    const float weightsMin = -0.393412f;
    const float weightsMax = 0.419856f;

    Buffer* weightsFixed = new Buffer(Dimensions(outputChannels, inputChannels), weightsMin, weightsMax, weightsBitsPerElement);

    weightsFixed->populateWithRandomValues(weightsMin, weightsMax);

//    uint16_t* weightData = (uint16_t*)(weightsFixed->_quantizedData);
//    const float min = weightsFixed->_min;
//    const float max = weightsFixed->_max;
//    const float range = ((max - min) / ((1 << weightsBitsPerElement) - 0.0f));
//    for (int index = 0; index < 16; index += 1) {
//      const uint16_t value = weightData[index];
//      const float floatValue = (min + (value * range));
//      fprintf(stderr, "weightData[%d] = 0x%08x, %d (%f)\n", index, value, value, floatValue);
//    }

weightsFixed->populateWithRandomValues(0.0f, 0.0f);

uint16_t* weightData = (uint16_t*)(weightsFixed->_quantizedData);
for (int whichQPU = 0; whichQPU < NUM_QPUS; whichQPU += 1) {
  uint16_t* weightRow = (weightData + (whichQPU * inputChannels));
  for (int index = 0; index < 16; index += 1) {
    weightRow[index] = ((16 * whichQPU) + index);
  }
}

//input->printContents();
weightsFixed->printContents();
//uint16_t* weightData = (uint16_t*)(weightsFixed->_quantizedData);
for (int index = 0; index < 16; index += 1) {
  if (index > 0) {
    fprintf(stderr, ", ");
  }
  fprintf(stderr, "%04x", weightData[index]);
}
fprintf(stderr, "\n");

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
      outputCPU->_data,
      ldc
    );

    qpu_cblas_sgemm_fixed(
      order,
      transposeA,
      transposeB,
      m,
      n,
      k,
      alpha,
      weightsFixed->_gpuMemoryBase,
      weightsMin,
      weightsMax,
      weightsBitsPerElement,
      lda,
      input->_gpuMemoryBase,
      ldb,
      beta,
      outputGPU->_gpuMemoryBase,
      ldc
    );
    delete weightsFixed;
  }

  outputCPU->printContents();
//  outputGPU->printContents();
  assert(buffer_are_all_close(outputCPU, outputGPU));

  delete outputCPU;
  delete outputGPU;
  delete input;
}

#endif // USE_QPU_GEMM
