//
//  convnode.cpp
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#include "convnode.h"

#include <assert.h>
#include <string.h>
#include <math.h>

#include "buffer.h"
#include "binary_format.h"
#include "matrix_ops.h"

ConvNode::ConvNode() : BaseNode(), _kernels(NULL), _bias(NULL), _areKernelsTransposed(false) {
  setClassName("ConvNode");
}

ConvNode::~ConvNode() {
  if (_kernels != NULL) {
    delete _kernels;
  }
  if (_bias != NULL) {
    delete _bias;
  }
}

Buffer* ConvNode::run(Buffer* input) {
  if (_output != NULL) {
    delete _output;
  }

  Dimensions inputDims = input->_dims;
  const int inputChannels = inputDims[inputDims._length - 1];
  const int valuesPerKernel = (inputChannels * _kernelWidth * _kernelWidth);
  if (_areKernelsTransposed) {
    Dimensions expectedKernelsDims(_kernelCount, valuesPerKernel);
    assert(expectedKernelsDims == _kernels->_dims);
  } else {
    Dimensions expectedKernelsDims(valuesPerKernel, _kernelCount);
    assert(expectedKernelsDims == _kernels->_dims);
  }

  Buffer* inputWithMargin;
  if (_marginSize == 0) {
    inputWithMargin = input;
  } else {
    inputWithMargin = matrix_insert_margin(input, _marginSize, _marginSize);
  }

  _output = matrix_correlate(inputWithMargin, _kernels, _kernelWidth, _kernelCount, _sampleStride, _areKernelsTransposed);
  _output->setName(_name);

  matrix_add_inplace(_output, _bias, 1.0);

  if (_marginSize != 0) {
    delete inputWithMargin;
  }

  return _output;
}

char* ConvNode::debugString() {
  char additionalInfo[MAX_DEBUG_STRING_LEN];
  snprintf(additionalInfo, sizeof(additionalInfo),
    "_kernelWidth=%d, _kernelCount=%d, _marginSize=%d, _sampleStride=%d, _kernels->_dims=%s, _bias->_dims=%s",
    _kernelWidth, _kernelCount, _marginSize, _sampleStride,
    _kernels->_dims.debugString(), _bias->_dims.debugString());
  return this->debugStringWithMessage(additionalInfo);
}

void ConvNode::saveDebugImage() {

  const jpfloat_t offset = 127.0f;
  const jpfloat_t scale = (_kernelWidth * _kernelWidth) * 16.0f;

  const int outputChannels = _kernelCount;
  const int inputChannels = 3;
  for (int outputChannel = 0; outputChannel < outputChannels; outputChannel += 1) {
    const Dimensions kernelDims(_kernelWidth, _kernelWidth, inputChannels);
    Buffer* kernelImage = new Buffer(kernelDims);
    const size_t maxNameLength = 1024;
    char name[maxNameLength];
    snprintf(name, maxNameLength, "%s_%03d", _name, outputChannel);
    kernelImage->setName(name);
    for (int kernelY = 0; kernelY < _kernelWidth; kernelY += 1) {
      for (int kernelX = 0; kernelX < _kernelWidth; kernelX += 1) {
        for (int kernelChannel = 0; kernelChannel < inputChannels; kernelChannel += 1) {
          const int kernelsOffset = (
            (kernelY * _kernelWidth * inputChannels * _kernelCount) +
            (kernelX * inputChannels * _kernelCount) +
            (kernelChannel * _kernelCount) +
            outputChannel);
          const jpfloat_t kernelValue = *(_kernels->_data + kernelsOffset);
          jpfloat_t* imageData = kernelImage->_data + kernelDims.offset(kernelY, kernelX, kernelChannel);
          *imageData = fmin(255.0f, fmax(0.0f, (offset + (kernelValue * scale))));
        }
      }
    }
    kernelImage->saveDebugImage();
    delete kernelImage;
  }
}

SBinaryTag* ConvNode::toTag() {
  SBinaryTag* resultDict = create_dict_tag();
  resultDict = add_string_to_dict(resultDict, "class", "conv");
  resultDict = add_string_to_dict(resultDict, "name", _name);

  SBinaryTag* specDict = create_dict_tag();
  specDict = add_uint_to_dict(specDict, "num_kernels", _kernelCount);
  specDict = add_uint_to_dict(specDict, "ksize", _kernelWidth);
  specDict = add_uint_to_dict(specDict, "stride", _sampleStride);
  resultDict = add_tag_to_dict(resultDict, "spec", specDict);
  free(specDict);

  const bool wantTransposedOutput = true;
  const int outputBitDepth = 16;

  if (wantTransposedOutput != _areKernelsTransposed) {
    _kernels->transpose(); // First transpose so they match
  }
  SBinaryTag* kernelsTag = buffer_to_tag_dict(_kernels, outputBitDepth);
  resultDict = add_tag_to_dict(resultDict, "kernels", kernelsTag);
  free(kernelsTag);
  if (wantTransposedOutput != _areKernelsTransposed) {
    _kernels->transpose(); // Undo the original transpose by applying another
  }

  if (wantTransposedOutput) {
    resultDict = add_uint_to_dict(resultDict, "are_kernels_transposed", 1);
  } else {
    resultDict = add_uint_to_dict(resultDict, "are_kernels_transposed", 0);
  }

  resultDict = add_uint_to_dict(resultDict, "has_bias", _useBias);
  if (_useBias) {
    SBinaryTag* biasTag = buffer_to_tag_dict(_bias);
    resultDict = add_tag_to_dict(resultDict, "bias", biasTag);
    free(biasTag);
  }

  resultDict = add_uint_to_dict(resultDict, "padding", _marginSize);

  return resultDict;
}

BaseNode* new_convnode_from_tag(SBinaryTag* tag, bool skipCopy) {
  const char* className = get_string_from_dict(tag, "class");
  assert(strcmp(className, "conv") == 0);
  ConvNode* result = new ConvNode();

  SBinaryTag* specDict = get_tag_from_dict(tag, "spec");
  result->_kernelCount = get_uint_from_dict(specDict, "num_kernels");
  result->_kernelWidth = get_uint_from_dict(specDict, "ksize");
  result->_sampleStride = get_uint_from_dict(specDict, "stride");

  SBinaryTag* kernelsTag = get_tag_from_dict(tag, "kernels");
  result->_kernels = buffer_from_tag_dict(kernelsTag, skipCopy);

  result->_useBias = (get_uint_from_dict(tag, "has_bias") != 0);
  if (result->_useBias) {
    SBinaryTag* biasTag = get_tag_from_dict(tag, "bias");
    result->_bias = buffer_from_tag_dict(biasTag, skipCopy);
    assert(result->_bias->_dims.elementCount() > 0);
  }

  result->_marginSize = get_uint_from_dict(tag, "padding");

  if (get_tag_from_dict(tag, "are_kernels_transposed")) {
    result->_areKernelsTransposed = get_uint_from_dict(tag, "are_kernels_transposed");
  }

  return result;
}
