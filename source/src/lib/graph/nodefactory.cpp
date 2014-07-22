//
//  nodefactory.cpp
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#include "nodefactory.h"

#include <string.h>

#include "basenode.h"
#include "buffer.h"
#include "convnode.h"
#include "dropoutnode.h"
#include "flatnode.h"
#include "gconvnode.h"
#include "neuronnode.h"
#include "normalizenode.h"
#include "poolnode.h"
#include "relunode.h"
#include "maxnode.h"

typedef BaseNode*(*nodeFunctionPtr)(SBinaryTag*, bool);
typedef struct SFuncLookupStruct {
  const char* className;
  nodeFunctionPtr createFunction;
} SFuncLookup;

static SFuncLookup g_createFunctions[] = {
  {"conv", new_convnode_from_tag},
  {"dropout", new_dropoutnode_from_tag},
  {"flat", new_flatnode_from_tag},
  {"gconv", new_gconvnode_from_tag},
  {"neuron", new_neuronnode_from_tag},
  {"normalize", new_normalizenode_from_tag},
  {"pool", new_poolnode_from_tag},
  {"relu", new_relunode_from_tag},
  {"max", new_maxnode_from_tag},
};
static int g_createFunctionsLength = (sizeof(g_createFunctions) / sizeof(g_createFunctions[0]));

BaseNode* new_node_from_tag(SBinaryTag* tag, bool skipCopy) {

  const char* tagClass = get_string_from_dict(tag, "class");
  nodeFunctionPtr createFunction = NULL;
  for (int index = 0; index < g_createFunctionsLength; index += 1) {
    SFuncLookup* entry = &g_createFunctions[index];
    if (strcmp(entry->className, tagClass) == 0) {
      createFunction = entry->createFunction;
    }
  }
  if (createFunction == NULL) {
    fprintf(stderr, "new_node_from_tag(): Couldn't find a factory function for node class '%s'\n", tagClass);
    return NULL;
  }
  BaseNode* result = createFunction(tag, skipCopy);
  const char* name = get_string_from_dict(tag, "name");
  result->setName(name);
  return result;
}
