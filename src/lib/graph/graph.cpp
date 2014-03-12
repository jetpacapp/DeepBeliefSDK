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
#define SAVE_RESULTS
#if defined(CHECK_RESULTS) || defined(SAVE_RESULTS)
#define FN_LEN (1024)
#define DUMP_FILE_PATH ("data/c_dog_blobs/")
#endif // CHECK_RESULTS || SAVE_RESULTS

Graph::Graph() :
  _useMemoryMap(false),
  _isHomebrewed(false),
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

Buffer* Graph::run(Buffer* input, int layerOffset) {

#ifdef DO_LOG_OPERATIONS
  fprintf(stderr, "Graph::run() input=%s\n", input->debugString());
#endif // DO_LOG_OPERATIONS

  Buffer* currentInput = input;
  const int howManyLayers = (_layersLength + layerOffset);
  for (int index = 0; index < howManyLayers; index += 1) {
    BaseNode* layer = _layers[index];
#ifdef CHECK_RESULTS
    char expectedInputFilename[FN_LEN];
    snprintf(expectedInputFilename, FN_LEN,
      "%s%03d_input_%s.blob",
      DUMP_FILE_PATH, ((index * 2) + 1), layer->_name);
    Buffer* expectedInput = buffer_from_dump_file(expectedInputFilename);
    const Dimensions& currentInputDims = currentInput->_dims;
    if (_isHomebrewed) {
      expectedInput->convertFromChannelMajor(currentInputDims);
    }
    if (expectedInput->canReshapeTo(currentInputDims)) {
      expectedInput->reshape(currentInputDims);
    }
    if (!buffer_are_all_close(currentInput, expectedInput)) {
      fprintf(stderr, "Inputs don't match for %s\n", layer->_name);
    } else {
      fprintf(stderr, "!!!!!Inputs match for %s\n", layer->_name);
    }
#endif // CHECK_RESULTS
#ifdef SAVE_RESULTS
    char inputFilename[FN_LEN];
    snprintf(inputFilename, FN_LEN,
      "%s%03d_input.blob",
      DUMP_FILE_PATH, ((index * 2) + 1));
    buffer_dump_to_file(currentInput, inputFilename);
#endif // SAVE_RESULTS

    Buffer* currentOutput = layer->run(currentInput);
    currentOutput->setName(layer->_name);

#ifdef DO_LOG_OPERATIONS
    fprintf(stderr, "Graph::run() currentOutput=%s\n", currentOutput->debugString());
#endif // DO_LOG_OPERATIONS

#ifdef CHECK_RESULTS
    char expectedOutputFilename[FN_LEN];
    snprintf(expectedOutputFilename, FN_LEN,
      "%s%03d_output_%s.blob",
      DUMP_FILE_PATH, ((index * 2) + 2), layer->_name);
    Buffer* expectedOutput = buffer_from_dump_file(expectedOutputFilename);
    const Dimensions& currentOutputDims = currentOutput->_dims;
    if (_isHomebrewed) {
      expectedOutput->convertFromChannelMajor(currentOutputDims);
    }
    if (expectedOutput->canReshapeTo(currentOutputDims)) {
      expectedOutput->reshape(currentOutputDims);
    }
    if (!buffer_are_all_close(currentOutput, expectedOutput)) {
      fprintf(stderr, "!!!Outputs don't match for %s\n", layer->_name);
    } else {
      fprintf(stderr, "***Outputs match for %s\n", layer->_name);
    }
#endif // CHECK_RESULTS

#ifdef SAVE_RESULTS
    char outputFilename[FN_LEN];
    snprintf(outputFilename, FN_LEN,
      "%s%03d_output.blob",
      DUMP_FILE_PATH, ((index * 2) + 1));
    buffer_dump_to_file(currentOutput, outputFilename);
#endif // SAVE_RESULTS
    currentInput = currentOutput;
  }

  return currentInput;
}

void Graph::printDebugOutput() {
  fprintf(stderr, "************************\nJPCNN Network with %d layers\n", _layersLength);
  for (int index = 0; index < _layersLength; index += 1) {
    BaseNode* layer = _layers[index];
    fprintf(stderr, "%s\n", layer->debugString());
  }
  fprintf(stderr, "************************\n");
}

Graph* new_graph_from_file(const char* filename, int useMemoryMap, int isHomebrewed) {

  SBinaryTag* graphDict = read_tag_from_file(filename, useMemoryMap);
  if (graphDict == NULL) {
    fprintf(stderr, "new_graph_from_file(): Couldn't interpret data from '%s'\n", filename);
    return NULL;
  }

  Graph* result = new Graph();

  result->_useMemoryMap = useMemoryMap;
  result->_isHomebrewed = isHomebrewed;
  result->_fileTag = graphDict;

  SBinaryTag* dataMeanTag = get_tag_from_dict(graphDict, "data_mean");
  assert(dataMeanTag != NULL);
  result->_dataMean = buffer_from_tag_dict(dataMeanTag, useMemoryMap);
  assert(result->_dataMean != NULL);

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

void save_graph_to_file(Graph* graph, const char* filename) {

  SBinaryTag* graphDict = create_dict_tag();

  SBinaryTag* dataMeanTag = buffer_to_tag_dict(graph->_dataMean);
  graphDict = add_tag_to_dict(graphDict, "data_mean", dataMeanTag);
  free(dataMeanTag);

  SBinaryTag* layersTag = create_list_tag();
  for (int index = 0; index < graph->_layersLength; index += 1) {
    BaseNode* node = graph->_layers[index];
    SBinaryTag* currentLayerTag = node->toTag();
    layersTag = add_tag_to_list(layersTag, currentLayerTag);
    free(currentLayerTag);
  }
  graphDict = add_tag_to_dict(graphDict, "layers", layersTag);
  free(layersTag);

  SBinaryTag* labelNamesTag = create_list_tag();
  for (int index = 0; index < graph->_labelNamesLength; index += 1) {
    labelNamesTag = add_string_to_list(labelNamesTag, graph->_labelNames[index]);
  }
  graphDict = add_tag_to_dict(graphDict, "label_names", labelNamesTag);
  free(labelNamesTag);

  SBinaryTag* copyrightTag = create_string_tag("Copyright Jetpac Inc., 2014");
  graphDict = add_tag_to_dict(graphDict, "copyright", copyrightTag);
  free(copyrightTag);

  FILE* outputFile = fopen(filename, "wb");
  assert(outputFile != NULL);
  fwrite(graphDict, (graphDict->length + 8), 1, outputFile);
  fclose(outputFile);
  free(graphDict);
}
