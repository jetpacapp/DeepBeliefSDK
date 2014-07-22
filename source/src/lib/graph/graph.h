//
//  graph.h
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#ifndef INCLUDE_GRAPH_H
#define INCLUDE_GRAPH_H

#include "jpcnn.h"
#include "binary_format.h"

class BaseNode;
class Buffer;

class Graph {
public:

  Graph();
  virtual ~Graph();

  Buffer* run(Buffer* input, int layerOffset = 0);
  void printDebugOutput();

  bool _useMemoryMap;
  bool _isHomebrewed;
  bool _isLibCCV;
  SBinaryTag* _fileTag;
  const char* _source;
  int _inputSize;

  Buffer* _dataMean;
  BaseNode* _preparationNode;
  BaseNode** _layers;
  int _layersLength;
  char** _labelNames;
  int _labelNamesLength;
};

Graph* new_graph_from_file(const char* filename, int useMemoryMap, int isHomebrewed);
void save_graph_to_file(Graph* graph, const char* filename);

#endif // INCLUDE_GRAPH_H