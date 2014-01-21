//
//  svmutils.cpp
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#include "svmutils.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

void init_growable_array(SGrowableArray* array, int initialAllocatedItems) {
  array->itemCount = 0;
  array->data = (float*)(malloc(sizeof(float) * initialAllocatedItems));
  array->allocatedItems = initialAllocatedItems;
}

void append_to_growable_array(SGrowableArray* array, float* newData, int newDataLength) {
  const int newFullLength = (array->itemCount + newDataLength);
  if (newFullLength > array->allocatedItems) {
    int newAllocatedItems = (array->allocatedItems * 2);
    while (newFullLength > newAllocatedItems) {
      newAllocatedItems *= 2;
    }
    array->data = (float*)(realloc(array->data, (sizeof(float) * newAllocatedItems)));
    array->allocatedItems = newAllocatedItems;
  }
  float* const oldDataEnd = (array->data + array->itemCount);
  memcpy(oldDataEnd, newData, (sizeof(float) * newDataLength));
  array->itemCount += newDataLength;
}

void free_growable_array_data(SGrowableArray* array) {
  free(array->data);
}

SLibSvmTrainingInfo* create_training_info() {
  SLibSvmTrainingInfo* result = (SLibSvmTrainingInfo*)(malloc(sizeof(SLibSvmTrainingInfo)));
  result->itemCount = 0;
  result->featuresPerItem = 0;
  init_growable_array(&result->itemLabels);
  init_growable_array(&result->itemFeatures);
  return result;
}

void add_features_to_training_info(SLibSvmTrainingInfo* trainingInfo, float label, float* features, int featuresCount) {
  // Is this the first item to be added?
  if (trainingInfo->itemCount == 0) {
    trainingInfo->featuresPerItem = featuresCount;
  } else {
    // All the items should have the same number of features
    assert(trainingInfo->featuresPerItem == featuresCount);
  }
  append_to_growable_array(&trainingInfo->itemLabels, &label, 1);
  append_to_growable_array(&trainingInfo->itemFeatures, features, featuresCount);
  trainingInfo->itemCount += 1;
}

void destroy_training_info(SLibSvmTrainingInfo* trainingInfo) {
  free_growable_array_data(&trainingInfo->itemLabels);
  free_growable_array_data(&trainingInfo->itemFeatures);
  free(trainingInfo);
}

SLibSvmProblem* create_svm_problem_from_training_info(SLibSvmTrainingInfo* trainingInfo) {
  const int itemCount = trainingInfo->itemCount;
  const int featuresPerItem = trainingInfo->featuresPerItem;
  const float* const allItemsFeatures = trainingInfo->itemFeatures.data;
  const float* allItemsLabels = trainingInfo->itemLabels.data;
  struct svm_problem* svmProblem = (struct svm_problem*)(malloc(sizeof(svm_problem)));
  svmProblem->l = trainingInfo->itemCount;
  svmProblem->y = (double*)(malloc(sizeof(double) * itemCount));
  svmProblem->x = (struct svm_node**)(malloc((sizeof(struct svm_node*) * itemCount)));
  const int nodesPerItem = (featuresPerItem + 1);
  const int totalNodeCount = (nodesPerItem * itemCount);
  struct svm_node* nodeData = (struct svm_node*)(malloc(sizeof(struct svm_node) * totalNodeCount));
  for (int itemIndex = 0; itemIndex < itemCount; itemIndex += 1) {
    svmProblem->y[itemIndex] = allItemsLabels[itemIndex];
    const float* itemFeatures = (allItemsFeatures + (itemIndex * featuresPerItem));
    struct svm_node* itemNodes = (nodeData + (itemIndex * nodesPerItem));
    svmProblem->x[itemIndex] = itemNodes;
    for (int featureIndex = 0; featureIndex < featuresPerItem; featureIndex += 1) {
      const float featureValue = itemFeatures[featureIndex];
      struct svm_node* currentNode = &itemNodes[featureIndex];
      currentNode->index = (featureIndex + 1);
      currentNode->value = featureValue;
    }
    struct svm_node* lastItemNode = (itemNodes + (nodesPerItem - 1));
    lastItemNode->index = -1;
  }
  SLibSvmProblem* result = (SLibSvmProblem*)(malloc(sizeof(SLibSvmProblem)));
  result->svmProblem = svmProblem;
  result->nodeData = nodeData;
  result->svmParameters = create_svm_parameters();
  result->svmParameters->gamma = (1.0 / featuresPerItem);
  return result;
}

void destroy_svm_problem(SLibSvmProblem* problem) {
  free(problem->svmProblem->y);
  free(problem->svmProblem->x);
  free(problem->svmProblem);
  free(problem->nodeData);
  destroy_svm_parameters(problem->svmParameters);
  free(problem);
}

struct svm_parameter* create_svm_parameters() {
  struct svm_parameter* result = (struct svm_parameter*)(malloc(sizeof(struct svm_parameter)));
	result->svm_type = C_SVC;
	result->kernel_type = RBF;
	result->degree = 3;
	result->gamma = 0;
	result->coef0 = 0;
	result->nu = 0.5;
	result->cache_size = 100;
	result->C = 1;
	result->eps = 1e-3;
	result->p = 0.1;
	result->shrinking = 1;
	result->probability = 1;
	result->nr_weight = 0;
	result->weight_label = NULL;
	result->weight = NULL;
  return result;
}

void destroy_svm_parameters(struct svm_parameter* svmParameters) {
  free(svmParameters);
}

struct svm_node* create_node_list(float* features, int featuresCount) {
  const int nodesCount = (featuresCount + 1);
  struct svm_node* nodes = (struct svm_node*)(malloc(sizeof(struct svm_node) * nodesCount));
  for (int featureIndex = 0; featureIndex < featuresCount; featureIndex += 1) {
    const float featureValue = features[featureIndex];
    struct svm_node* currentNode = &nodes[featureIndex];
    currentNode->index = (featureIndex + 1);
    currentNode->value = featureValue;
  }
  struct svm_node* lastItemNode = (nodes + (nodesCount - 1));
  lastItemNode->index = -1;
  return nodes;
}

void destroy_node_list(struct svm_node* nodeList) {
  free(nodeList);
}


