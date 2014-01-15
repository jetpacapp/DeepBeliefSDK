//
//  graph.cpp
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#include "graph.h"

#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "buffer.h"
#include "binary_format.h"
#include "basenode.h"
#include "nodefactory.h"

//#define CHECK_RESULTS
#ifdef CHECK_RESULTS
#define FN_LEN (1024)
#endif // CHECK_RESULTS

Graph::Graph() :
  _useMemoryMap(false),
  _fileTag(NULL),
  _dataMean(NULL),
  _preparationNode(NULL),
  _layers(NULL),
  _layersLength(0),
  _labelNames(NULL),
  _labelNamesLength(0) {
}

Graph::~Graph() {
  if (_fileTag != NULL) {
    deallocate_file_tag(_fileTag, _useMemoryMap);
  }
  if (_dataMean != NULL) {
    delete _dataMean;
  }
  if (_preparationNode != NULL) {
    delete _preparationNode;
  }
  if (_layers != NULL) {
    for (int index = 0; index < _layersLength; index += 1) {
      delete _layers[index];
    }
    free(_layers);
  }
  if (_labelNames != NULL) {
    for (int index = 0; index < _labelNamesLength; index += 1) {
      free(_labelNames[index]);
    }
    free(_labelNames);
  }
}

Buffer* Graph::run(Buffer* input) {

  Buffer* currentInput = input;
  for (int index = 0; index < _layersLength; index += 1) {
    BaseNode* layer = _layers[index];
#ifdef CHECK_RESULTS
    char expectedInputFilename[FN_LEN];
    snprintf(expectedInputFilename, FN_LEN,
      "data/lena_blobs/%03d_input_%s.blob",
      ((index * 2) + 1), layer->_name);
    Buffer* expectedInput = buffer_from_dump_file(expectedInputFilename);
    if (!buffer_are_all_close(currentInput, expectedInput)) {
      fprintf(stderr, "Inputs don't match for %s\n", layer->_name);
    }
#endif // CHECK_RESULTS
    Buffer* currentOutput = layer->run(currentInput);
#ifdef CHECK_RESULTS
    char expectedOutputFilename[FN_LEN];
    snprintf(expectedOutputFilename, FN_LEN,
      "data/lena_blobs/%03d_output_%s.blob",
      ((index * 2) + 2), layer->_name);
    Buffer* expectedOutput = buffer_from_dump_file(expectedOutputFilename);
    if (!buffer_are_all_close(currentOutput, expectedOutput)) {
      fprintf(stderr, "!!!Outputs don't match for %s\n", layer->_name);
    } else {
      fprintf(stderr, "***Outputs match for %s\n", layer->_name);
    }
#endif // CHECK_RESULTS
    currentInput = currentOutput;
  }

  return currentInput;
}


Graph* new_graph_from_file(const char* filename, int useMemoryMap) {

  SBinaryTag* graphDict = read_tag_from_file(filename, useMemoryMap);
  if (graphDict == NULL) {
    fprintf(stderr, "new_graph_from_file(): Couldn't interpret data from '%s'\n", filename);
    return NULL;
  }

  Graph* result = new Graph();

  result->_useMemoryMap = useMemoryMap;
  result->_fileTag = graphDict;

  SBinaryTag* dataMeanTag = get_tag_from_dict(graphDict, "data_mean");
  assert(dataMeanTag != NULL);
  result->_dataMean = buffer_from_tag_dict(dataMeanTag, useMemoryMap);

  SBinaryTag* layersTag = get_tag_from_dict(graphDict, "layers");
  assert(layersTag != NULL);
  result->_layersLength = count_list_entries(layersTag);
  result->_layers = (BaseNode**)(malloc(sizeof(BaseNode*) * result->_layersLength));

  int index = 0;
  SBinaryTag* currentLayerTag = get_first_list_entry(layersTag);
  while (currentLayerTag != NULL) {
    BaseNode* layerNode = new_node_from_tag(currentLayerTag, useMemoryMap);
    result->_layers[index] = layerNode;
    index += 1;
    currentLayerTag = get_next_list_entry(layersTag, currentLayerTag);
  }

  SBinaryTag* labelNamesTag = get_tag_from_dict(graphDict, "label_names");

  result->_labelNamesLength = count_list_entries(labelNamesTag);
  result->_labelNames = (char**)(malloc(sizeof(char*) * result->_labelNamesLength));

  index = 0;
  SBinaryTag* currentLabelNameTag = get_first_list_entry(labelNamesTag);
  while (currentLabelNameTag != NULL) {
    const char* const fileLabelName = currentLabelNameTag->payload.jpchar;
    const size_t labelNameLength = strlen(fileLabelName);
    result->_labelNames[index] = (char*)(malloc(labelNameLength + 1));
    strncpy(result->_labelNames[index], fileLabelName, labelNameLength);
    index += 1;
    currentLabelNameTag = get_next_list_entry(labelNamesTag, currentLabelNameTag);
  }

  return result;
}
