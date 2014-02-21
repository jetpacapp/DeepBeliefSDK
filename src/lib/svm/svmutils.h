//
//  svmutils.h
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#ifndef INCLUDE_SVMUTILS_H
#define INCLUDE_SVMUTILS_H

#include "svm.h"

typedef struct SGrowableArrayStruct {
  float* data;
  int itemCount;
  int allocatedItems;
} SGrowableArray;

typedef struct SLibSvmTrainingInfoStruct {
  int itemCount;
  int featuresPerItem;
  SGrowableArray itemLabels;
  SGrowableArray itemFeatures;
} SLibSvmTrainingInfo;

typedef struct SLibSvmProblemStruct {
  struct svm_problem* svmProblem;
  struct svm_parameter* svmParameters;
  struct svm_node* nodeData;
} SLibSvmProblem;

void init_growable_array(SGrowableArray* array, int initialAllocatedItems = 1024);
void append_to_growable_array(SGrowableArray* array, float* newData, int newDataLength);
void free_growable_array_data(SGrowableArray* array);

SLibSvmTrainingInfo* create_training_info();
void add_features_to_training_info(SLibSvmTrainingInfo* trainingInfo, float label, float* features, int featuresCount);
void destroy_training_info(SLibSvmTrainingInfo* trainingInfo);
SLibSvmProblem* create_svm_problem_from_training_info(SLibSvmTrainingInfo* trainingInfo);
void destroy_svm_problem(SLibSvmProblem* problem);
struct svm_parameter* create_svm_parameters();
void destroy_svm_parameters(struct svm_parameter* svmParameters);
struct svm_node* create_node_list(float* features, int featuresCount);
void destroy_node_list(struct svm_node* nodeList);

#endif // INCLUDE_SVMUTILS_H
