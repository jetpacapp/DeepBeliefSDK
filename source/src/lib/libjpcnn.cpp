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
#include "svmutils.h"
#include "glgemm.h"

typedef struct SPredictorInfoStruct {
  struct svm_model* model;
  SLibSvmProblem* problem;
} SPredictorInfo;

extern void test_qpu_gemm();

extern "C" {

void* jpcnn_create_network(const char* filename) {
//test_qpu_gemm();
//exit(1);
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

void jpcnn_classify_image(void* networkHandle, void* inputHandle, unsigned int flags, int layerOffset, float** outPredictionsValues, int* outPredictionsLength, char*** outPredictionsNames, int* outPredictionsNamesLength) {

  const bool doMultiSample = (flags & JPCNN_MULTISAMPLE);
  const bool doRandomSample = (flags & JPCNN_RANDOM_SAMPLE);

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
  const int rescaledSize = graph->_inputSize;

  PrepareInput prepareInput(graph->_dataMean, !doMultiSample, doFlip, doRandomSample, imageSize, rescaledSize, isMeanChanneled);
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

void* jpcnn_create_trainer() {
  SLibSvmTrainingInfo* trainer = create_training_info();
  return trainer;
}

void jpcnn_destroy_trainer(void* trainerHandle) {
  SLibSvmTrainingInfo* trainer = (SLibSvmTrainingInfo*)(trainerHandle);
  destroy_training_info(trainer);
}

void jpcnn_train(void* trainerHandle, float expectedLabel, float* predictions, int predictionsLength) {
  SLibSvmTrainingInfo* trainer = (SLibSvmTrainingInfo*)(trainerHandle);
  add_features_to_training_info(trainer, expectedLabel, predictions, predictionsLength);
}

void* jpcnn_create_predictor_from_trainer(void* trainerHandle) {
  SLibSvmTrainingInfo* trainer = (SLibSvmTrainingInfo*)(trainerHandle);
  SLibSvmProblem* problem = create_svm_problem_from_training_info(trainer);
  const char* parameterCheckError = svm_check_parameter(problem->svmProblem, problem->svmParameters);
  if (parameterCheckError != NULL) {
    fprintf(stderr, "libsvm parameter check error: %s\n", parameterCheckError);
    destroy_svm_problem(problem);
    return NULL;
  }
  struct svm_model* model = svm_train(problem->svmProblem, problem->svmParameters);
  SPredictorInfo* result = (SPredictorInfo*)(malloc(sizeof(SPredictorInfo)));
  result->model = model;
  result->problem = problem;
  return result;
}

void jpcnn_destroy_predictor(void* predictorHandle) {
  SPredictorInfo* predictorInfo = (SPredictorInfo*)(predictorHandle);
  svm_free_and_destroy_model(&predictorInfo->model);
  if (predictorInfo->problem != NULL) {
    destroy_svm_problem(predictorInfo->problem);
  }
  free(predictorInfo);
}

int jpcnn_save_predictor(const char* filename, void* predictorHandle) {
  SPredictorInfo* predictorInfo = (SPredictorInfo*)(predictorHandle);
  struct svm_model* model = predictorInfo->model;
  const int saveResult = svm_save_model(filename, model);
  if (saveResult != 0) {
    fprintf(stderr, "Couldn't save libsvm model file to '%s'\n", filename);
    return 0;
  }
  return 1;
}

void* jpcnn_load_predictor(const char* filename) {
  struct svm_model* model = svm_load_model(filename);
  SPredictorInfo* result = (SPredictorInfo*)(malloc(sizeof(SPredictorInfo)));
  result->model = model;
  result->problem = NULL;
  return result;
}

void jpcnn_print_predictor(void* predictorHandle) {
  SPredictorInfo* predictorInfo = (SPredictorInfo*)(predictorHandle);
  struct svm_model* model = predictorInfo->model;
  const int saveResult = svm_save_model_to_file_handle(stderr, model);
  if (saveResult != 0) {
    fprintf(stderr, "Couldn't print libsvm model file to stderr\n");
  }
}

float jpcnn_predict(void* predictorHandle, float* predictions, int predictionsLength) {
  SPredictorInfo* predictorInfo = (SPredictorInfo*)(predictorHandle);
  struct svm_model* model = predictorInfo->model;
  struct svm_node* nodes = create_node_list(predictions, predictionsLength);
  double probabilityEstimates[2];
  svm_predict_probability(model, nodes, probabilityEstimates);
  const double predictionValue = probabilityEstimates[0];
  destroy_node_list(nodes);
  return predictionValue;
}


}