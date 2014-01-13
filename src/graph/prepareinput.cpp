//
//  prepareinput.cpp
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#include "prepareinput.h"

#include "math.h"
#include "assert.h"
#include "string.h"

#include "buffer.h"
#include "matrix_ops.h"

const int kOutputWidth = 227;
const int kOutputHeight = 227;
const int kOutputChannels = 3;

const int kRescaledWidth = 256;
const int kRescaledHeight = 256;

static void rescale_image_to_fit(Buffer* input, Buffer* output, bool doFlip);
static void crop_and_flip_image(Buffer* destBuffer, Buffer* sourceBuffer, int offsetX, int offsetY, bool doFlipHorizontal);

PrepareInput::PrepareInput(Buffer* dataMean, bool useCenterOnly) {
  Dimensions expectedDims(kRescaledHeight, kRescaledWidth, kOutputChannels);
  assert(expectedDims == dataMean->_dims);
  _dataMean = dataMean;
  _useCenterOnly = useCenterOnly;
  setClassName("PrepareInput");
}

PrepareInput::~PrepareInput() {
  // Do nothing
}

Buffer* PrepareInput::run(Buffer* input) {

  Dimensions rescaledDims(kRescaledHeight, kRescaledWidth, kOutputChannels);
  Buffer* rescaled = new Buffer(rescaledDims);
  rescaled->setName("rescaled");

  rescale_image_to_fit(input, rescaled, true);
  rescaled->saveDebugImage();
  add_matrix_inplace(rescaled, _dataMean, -1.0f);

  if (_output != NULL) {
    delete _output;
  }
  const int deltaX = (kRescaledWidth - kOutputWidth);
  const int deltaY = (kRescaledHeight - kOutputHeight);
  const int marginX = (deltaX / 2);
  const int marginY = (deltaY / 2);

  if (_useCenterOnly) {

    Dimensions outputDims(1, kOutputHeight, kOutputWidth, kOutputChannels);
    _output = new Buffer(outputDims);
    _output->setName("prepareInput_output");

    const int sourceX = marginX;
    const int sourceY = marginY;

    Buffer* blitDestination = buffer_view_at_top_index(_output, 0);
    crop_and_flip_image(blitDestination, rescaled, sourceX, sourceY, false);

  } else {

    Dimensions outputDims(10, kOutputHeight, kOutputWidth, kOutputChannels);
    _output = new Buffer(outputDims);
    _output->setName("prepareInput_output");

    for (int flipPass = 0; flipPass < 2; flipPass += 1) {
      const bool doFlip = (flipPass == 1);
      for (int yIndex = 0; yIndex < 2; yIndex += 1) {
        for (int xIndex = 0; xIndex < 2; xIndex += 1) {
          const int viewIndex = ((flipPass * 5) + (yIndex * 2) + xIndex);
          Buffer* blitDestination = buffer_view_at_top_index(_output, viewIndex);

          const int sourceX = (xIndex * deltaX);
          const int sourceY = (yIndex * deltaY);

          crop_and_flip_image(blitDestination, rescaled, sourceX, sourceY, doFlip);
        }
      }
    }
  }

  return _output;
}

void rescale_image_to_fit(Buffer* input, Buffer* output, bool doFlip) {

  const Dimensions inputDims = input->_dims;
  const int inputWidth = inputDims[1];
  const int inputHeight = inputDims[0];
  const int inputChannels = inputDims[2];

  const Dimensions outputDims = output->_dims;
  const int outputWidth = outputDims[1];
  const int outputHeight = outputDims[0];
  const int outputChannels = outputDims[2];

  const float flipBias = (doFlip) ? inputHeight : 0.0f;
  const float flipScale = (doFlip) ? -1.0f : 1.0f;

  const float scaleX = (inputWidth / (jpfloat_t)(outputWidth));
  const float scaleY = (flipScale * (inputHeight / (jpfloat_t)(outputHeight)));
  const int channelsToWrite = MIN(outputChannels, inputChannels);

  const Dimensions inputRowDims = inputDims.removeDimensions(1);
  const Dimensions outputRowDims = outputDims.removeDimensions(1);

  const jpfloat_t* inputDataStart = input->_data;
  jpfloat_t* const outputDataStart = output->_data;

  for (int outputY = 0; outputY < outputHeight; outputY += 1) {
    const jpfloat_t inputY = (flipBias + (outputY * scaleY));
    const int indexY0 = fmaxf(0.0f, floorf(inputY));
    const int indexY1 = fminf((inputHeight - 1.0f), ceilf(inputY));
    const jpfloat_t lerpY = (indexY1 - inputY);
    const jpfloat_t oneMinusLerpY = (1.0f - lerpY);

    const int inputRowY0Offset = inputDims.offset(indexY0, 0, 0);
    const jpfloat_t* const inputRowY0 = (inputDataStart + inputRowY0Offset);
    const int inputRowY1Offset = inputDims.offset(indexY1, 0, 0);
    const jpfloat_t* const inputRowY1 = (inputDataStart + inputRowY1Offset);
    const int outputRowOffset = outputDims.offset(outputY, 0, 0);
    jpfloat_t* const outputRow = (outputDataStart + outputRowOffset);

    for (int outputX = 0; outputX < outputWidth; outputX += 1) {
      const jpfloat_t inputX = (outputX * scaleX);
      const int indexX0 = fmaxf(0.0f, floorf(inputX));
      const int indexX1 = fminf((inputWidth - 1.0f), ceilf(inputX));
      const jpfloat_t lerpX = (indexX1 - inputX);
      const jpfloat_t oneMinusLerpX = (1.0f - lerpX);

      const int indexX0Offset = inputRowDims.offset(indexX0, 0);
      const int indexX1Offset = inputRowDims.offset(indexX1, 0);

      const jpfloat_t* const input00Base = (inputRowY0 + indexX0Offset);
      const jpfloat_t* const input01Base = (inputRowY0 + indexX1Offset);
      const jpfloat_t* const input10Base = (inputRowY1 + indexX0Offset);
      const jpfloat_t* const input11Base = (inputRowY1 + indexX1Offset);

      const int outputOffset = outputRowDims.offset(outputX, 0);
      jpfloat_t* const outputBase = (outputRow + outputOffset);

      for (int channel = 0; channel < outputChannels; channel += 1) {
        jpfloat_t* const outputLocation = (outputBase + channel);
        if (channel >= channelsToWrite) {
          *outputLocation = 0.0f;
        } else {
          const jpfloat_t* input00Location = (input00Base + channel);
          const jpfloat_t* input01Location = (input01Base + channel);
          const jpfloat_t* input10Location = (input10Base + channel);
          const jpfloat_t* input11Location = (input11Base + channel);

          const jpfloat_t input00 = (*input00Location);
          const jpfloat_t input01 = (*input01Location);
          const jpfloat_t input10 = (*input10Location);
          const jpfloat_t input11 = (*input11Location);

          const jpfloat_t yInterp0 = ((input00 * lerpY) + (input10 * oneMinusLerpY));
          const jpfloat_t yInterp1 = ((input01 * lerpY) + (input11 * oneMinusLerpY));
          const jpfloat_t interp = ((yInterp0 * lerpX) + (yInterp1 * oneMinusLerpX));
          *outputLocation = interp;
        }
      }

    }
  }
}

void crop_and_flip_image(Buffer* destBuffer, Buffer* sourceBuffer, int offsetX, int offsetY, bool doFlipHorizontal) {

  const Dimensions destDims = destBuffer->_dims;
  const Dimensions sourceDims = sourceBuffer->_dims;
  assert((destDims._length == 3) && (sourceDims._length == 3));

  const int destWidth = destDims[1];
  const int destHeight = destDims[0];
  const int destChannels = destDims[2];

  const int sourceWidth = sourceDims[1];
  const int sourceHeight = sourceDims[0];
  const int sourceChannels = sourceDims[2];
  assert(destChannels == sourceChannels);

  const int sourceEndX = (offsetX + destWidth);
  assert(sourceEndX <= sourceWidth);
  const int sourceEndY = (offsetY + destHeight);
  assert(sourceEndY <= sourceHeight);

  const Dimensions destRowDims = destDims.removeDimensions(1);
  const int destRowElementCount = destRowDims.elementCount();
  const size_t destRowByteCount = (destRowElementCount * sizeof(jpfloat_t));

  jpfloat_t* const destDataStart = destBuffer->_data;
  jpfloat_t* const sourceDataStart = sourceBuffer->_data;

  if (!doFlipHorizontal) {
    for (int destY = 0; destY < destHeight; destY += 1) {
      const int sourceX = offsetX;
      const int sourceY = (destY + offsetY);
      const int sourceOffset = sourceDims.offset(sourceY, sourceX, 0);
      jpfloat_t* const sourceData = (sourceDataStart + sourceOffset);
      const int destOffset = destDims.offset(destY, 0, 0);
      jpfloat_t* const destData = (destDataStart + destOffset);
      memcpy(destData, sourceData, destRowByteCount);
    }
  } else {
    for (int destY = 0; destY < destHeight; destY += 1) {
      const int sourceX = offsetX;
      const int sourceY = (destY + offsetY);
      const int sourceOffset = sourceDims.offset(sourceY, sourceX, 0);
      jpfloat_t* const sourceLeft = (sourceDataStart + sourceOffset);
      const int destOffset = destDims.offset(destY, 0, 0);
      jpfloat_t* const destLeft = (destDataStart + destOffset);
      jpfloat_t* const destRight = (destLeft + destRowElementCount);
      jpfloat_t* destCurrent = destRight;
      jpfloat_t* sourceCurrent = sourceLeft;
      while (destCurrent != destLeft) {
        *destCurrent = *sourceCurrent;
        destCurrent -= 1;
        sourceCurrent += 1;
      }
    }
  }

}

