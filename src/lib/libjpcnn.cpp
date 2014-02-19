//
//  libjpcnn.cpp
//  jpcnn
//
//  Implements the external library interface to the Jetpac CNN code.
//
//  Created by Peter Warden on 1/15/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#include "libjpcnn.h"

#include <stdio.h>
#include <sys/time.h>

#include "buffer.h"
#include "prepareinput.h"
#include "graph.h"

extern "C" {

void* jpcnn_create_network(const char* filename) {
  Graph* graph = new_graph_from_file(filename, false, true);
  return (void*)(graph);
}

void jpcnn_destroy_network(void* networkHandle) {
  Graph* graph = (Graph*)(networkHandle);
  delete graph;
}

void* jpcnn_create_image_buffer_from_file(const char* filename) {
  Buffer* image = buffer_from_image_file(filename);
  return (void*)(image);
}

void jpcnn_destroy_image_buffer(void* imageHandle) {
  Buffer* image = (Buffer*)(imageHandle);
  delete image;
}

void* jpcnn_create_image_buffer_from_uint8_data(unsigned char* pixelData, int width, int height, int channels, int rowBytes, int reverseOrder, int doRotate) {
  const Dimensions imageDims(height, width, channels);
  Buffer* image = new Buffer(imageDims);
  unsigned char* const sourceDataStart = pixelData;
  jpfloat_t* const destDataStart = image->_data;
  const int valuesPerRow = (width * channels);

  if (doRotate) {
    for (int y = 0; y < height; y += 1) {
      jpfloat_t* dest = (destDataStart + (y * valuesPerRow));
      for (int x = 0; x < height; x += 1) {
        unsigned char* source = (sourceDataStart + (x * rowBytes) + (y * channels));
        if (reverseOrder) {
          jpfloat_t* destCurrent = (dest + (channels - 1));
          while (destCurrent >= dest) {
            *destCurrent = (jpfloat_t)(*source);
            destCurrent -= 1;
            source += 1;
          }
          dest += channels;
        } else {
          jpfloat_t* destEnd = (dest + channels);
          while (dest < destEnd) {
            *dest = (jpfloat_t)(*source);
            dest += 1;
            source += 1;
          }
        }
      }
    }
  } else {

    for (int y = 0; y < height; y += 1) {
      unsigned char* source = (sourceDataStart + (y * rowBytes));
      unsigned char* const sourceEnd = (source + valuesPerRow);
      jpfloat_t* dest = (destDataStart + (y * valuesPerRow));
      if (reverseOrder) {
        while (source < sourceEnd) {
          jpfloat_t* destCurrent = (dest + (channels - 1));
          while (destCurrent >= dest) {
            *destCurrent = (jpfloat_t)(*source);
            destCurrent -= 1;
            source += 1;
          }
          dest += channels;
        }
      } else {
        while (source < sourceEnd) {
          *dest = (jpfloat_t)(*source);
          dest += 1;
          source += 1;
        }
      }
    }
  }

  return (void*)(image);
}

void jpcnn_classify_image(void* networkHandle, void* inputHandle, int doMultiSample, int layerOffset, float** outPredictionsValues, int* outPredictionsLength, char*** outPredictionsNames, int* outPredictionsNamesLength) {

  Graph* graph = (Graph*)(networkHandle);
  Buffer* input = (Buffer*)(inputHandle);

  bool doFlip;
  int imageSize;
  bool isMeanChanneled;
  if (graph->_isHomebrewed) {
    imageSize = 224;
    doFlip = false;
    isMeanChanneled = true;
  } else {
    imageSize = 227;
    doFlip = true;
    isMeanChanneled = false;
  }
  const int rescaledSize = 256;

  PrepareInput prepareInput(graph->_dataMean, !doMultiSample, doFlip, imageSize, rescaledSize, isMeanChanneled);
  Buffer* rescaledInput = prepareInput.run(input);
  Buffer* predictions = graph->run(rescaledInput, layerOffset);

  *outPredictionsValues = predictions->_data;
  *outPredictionsLength = predictions->_dims.elementCount();
  if (layerOffset == 0) {
    *outPredictionsNames = graph->_labelNames;
    *outPredictionsNamesLength = graph->_labelNamesLength;
  } else {
    *outPredictionsNames = NULL;
    *outPredictionsNamesLength = predictions->_dims.removeDimensions(1).elementCount();
  }
}

void jpcnn_print_network(void* networkHandle) {
  Graph* graph = (Graph*)(networkHandle);
  if (graph == NULL) {
    fprintf(stderr, "jpcnn_print_network() - networkHandle is NULL\n");
    return;
  }
  graph->printDebugOutput();
}

}