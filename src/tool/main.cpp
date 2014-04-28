//
//  main.cpp
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#include <stdio.h>
#include <sys/time.h>
#include <string.h>
#include <getopt.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <stdint.h>

#include "libjpcnn.h"

#define STATIC_ARRAY_LEN(x) (sizeof(x) / sizeof(x[0]))

enum EToolMode {eSingleImage, eLibSvmTrain, eLibSvmTest, eLibSvmPredict};
typedef struct SToolArgumentValuesStruct {
  const char* networkFilename;
  const char* inputImageFilename;
  const char* modelFilename;
  const char* positiveDirectory;
  const char* negativeDirectory;
  const char* inputDirectory;
  const char* outputDirectory;
  int doMultisample;
  EToolMode mode;
  int doTime;
  float threshold;
  int layerOffset;
  int doDebugLogging;
} SToolArgumentValues;

typedef struct SToolOptionStruct {
  const char* longName;
  const char shortName;
  int isRequired;
  int hasArgument;
  const char* defaultValue;
  const char* description;
} SToolOption;

typedef void (*ClassifyImagesFunctionPtr)(void* cookie, float* predictions, int predictionsLength, const char* basename, const char* directoryName, const char* fullPath);

typedef struct STrainingCookieStruct {
  float label;
  void* trainer;
} STrainingCookie;

typedef struct STestingCookieStruct {
  float expectedLabel;
  void* predictor;
  float threshold;
  int truePositiveCount;
  int falsePositiveCount;
  int trueNegativeCount;
  int falseNegativeCount;
  int total;
} STestingCookie;

typedef struct SPredictionCookieStruct {
  const char* outputDirectory;
  void* predictor;
} SPredictionCookie;

static void parse_command_line_args(int argc, const char* argv[], SToolArgumentValues* outValues);
static void print_usage_and_exit(int argc, const char* argv[]);
static void do_classify_image(void* network, const char* inputFilename, int doMultisample, int layerOffset, float** predictions, int* predictionsLength, char*** predictionsLabels, long* outDuration);
static int has_image_suffix(const char* basename);
static void classify_images_in_directory(void* network, const char* directoryName, SToolArgumentValues* argValues, ClassifyImagesFunctionPtr callback, void* callbackCookie);
static void training_callback(void* cookie, float* predictions, int predictionsLength, const char* basename, const char* directoryName, const char* fullPath);
static void testing_callback(void* cookie, float* predictions, int predictionsLength, const char* basename, const char* directoryName, const char* fullPath);
static void prediction_callback(void* cookie, float* predictions, int predictionsLength, const char* basename, const char* directoryName, const char* fullPath);

static SToolOption g_toolOptions[] = {
  {"network", 'n', 1, 1, NULL, "The path to the neural network parameter file."},
  {"mode", 'm', 1, 1, NULL, "Which operation to perform. Can be 'single'/'s' to analyze one image, 'train'/'t' to produce a prediction model from folders of positive and negative images, 'test'/'e' to load a previously-created prediction model and run it against known positive and negative images, or 'predict'/'p' to run a prediction model against a folder of images."},
  {"input", 'i', 0, 1, "", "The path to a single input image."},
  {"positive", 'p', 0, 1, NULL, "The path to a folder of positive images."},
  {"negative", 'e', 0, 1, NULL, "The path to a folder of negative images."},
  {"multisample", 's', 0, 0, "0", "Whether to use a higher-quality but slower strategy of running the detection against ten different transforms of the image."},
  {"time", 't', 0, 0, "0", "Whether to print the time taken by the classification algorithm to stderr."},
  {"model", 'o', 0, 1, NULL, "The prediction model file."},
  {"threshold", 'h', 0, 1, "0.5", "Tunes the sensitivity of the prediction, with extreme values of 0.0 (accepts everything) to 1.0 (accepts nothing)."},
  {"layer", 'l', 0, 1, "0", "If specified, use a lower layer from the neural network."},
  {"inputdir", 'i', 0, 1, "", "The path to a folder containing images to run the predict mode analysis against."},
  {"outputdir", 'i', 0, 1, "", "The path to a folder that will be filled with symbolic links to the predict mode input files, with the predicted value as the sortable prefix to the file name."},
  {"debug", 'd', 0, 0, "0", "Whether to log extra debug information."},
};
const int g_toolOptionsLength = STATIC_ARRAY_LEN(g_toolOptions);

void parse_command_line_args(int argc, const char* argv[], SToolArgumentValues* outValues) {

  const char* optionStringValues[g_toolOptionsLength];
  for (int index = 0; index < g_toolOptionsLength; index += 1) {
    SToolOption* toolOption = &g_toolOptions[index];
    optionStringValues[index] = toolOption->defaultValue;
  }

  int argIndex = 1;
  while (argIndex < argc) {

    const char* fullArg = argv[argIndex];
    const size_t fullArgLength = strlen(fullArg);
    const char firstChar = fullArg[0];
    if ((firstChar != '-') || (fullArgLength < 2)) {
      // This is an anonymous argument (eg a file name or single '-', for now just ignore it
      argIndex += 1;
      continue;
    } else {
      const char secondChar = fullArg[1];
      SToolOption* foundToolOption = NULL;
      int foundIndex = -1;
      if (secondChar == '-') {
        // It's a long name, starting with '--'
        const char* longName = (fullArg + 2);
        for (int index = 0; index < g_toolOptionsLength; index += 1) {
          SToolOption* toolOption = &g_toolOptions[index];
          if (strcasecmp(toolOption->longName, longName) == 0) {
            foundToolOption = toolOption;
            foundIndex = index;
            break;
          }
        }
      } else {
        if (fullArgLength > 2) {
          fprintf(stderr, "Single-dash argument found with multiple characters: %s\n", fullArg);
          print_usage_and_exit(argc, argv);
        }
        for (int index = 0; index < g_toolOptionsLength; index += 1) {
          SToolOption* toolOption = &g_toolOptions[index];
          if (secondChar == toolOption->shortName) {
            foundToolOption = toolOption;
            foundIndex = index;
            break;
          }
        }
      }
      if (foundToolOption == NULL) {
        fprintf(stderr, "Unknown option '%s'\n", fullArg);
        print_usage_and_exit(argc, argv);
      }
      const bool hasArgument = foundToolOption->hasArgument;
      const char* valueString;
      if (hasArgument) {
        if (argIndex == (argc - 1)) {
          fprintf(stderr, "Missing argument for '%s'\n", fullArg);
          print_usage_and_exit(argc, argv);
        }
        valueString = argv[argIndex + 1];
      } else {
        valueString = "1";
      }
      optionStringValues[foundIndex] = valueString;
      if (hasArgument) {
        argIndex += 2;
      } else {
        argIndex += 1;
      }
    }
  }

  for (int index = 0; index < g_toolOptionsLength; index += 1) {
    const char* optionStringValue = optionStringValues[index];
    SToolOption* toolOption = &g_toolOptions[index];
    if (toolOption->isRequired && (optionStringValue == NULL)) {
      fprintf(stderr, "Missing option --%s/-%c\n", toolOption->longName, toolOption->shortName);
      print_usage_and_exit(argc, argv);
    }
  }

  for (int index = 0; index < g_toolOptionsLength; index += 1) {
    const char* optionStringValue = optionStringValues[index];
    SToolOption* toolOption = &g_toolOptions[index];
    const char* longName = toolOption->longName;
    if (strcmp("network", longName) == 0) {
      outValues->networkFilename = optionStringValue;
    } else if (strcmp("mode", longName) == 0) {
      if ((strcasecmp("single", optionStringValue) == 0) ||
        (strcasecmp("s", optionStringValue) == 0)) {
        outValues->mode = eSingleImage;
      } else if ((strcasecmp("train", optionStringValue) == 0) ||
        (strcasecmp("t", optionStringValue) == 0)) {
        outValues->mode = eLibSvmTrain;
      } else if ((strcasecmp("test", optionStringValue) == 0) ||
        (strcasecmp("e", optionStringValue) == 0)) {
        outValues->mode = eLibSvmTest;
      } else if ((strcasecmp("predict", optionStringValue) == 0) ||
        (strcasecmp("p", optionStringValue) == 0)) {
        outValues->mode = eLibSvmPredict;
      } else {
        fprintf(stderr, "Unknown argument to --mode/-m: '%s'\n", optionStringValue);
        print_usage_and_exit(argc, argv);
      }
    } else if (strcmp("input", longName) == 0) {
      outValues->inputImageFilename = optionStringValue;
    } else if (strcmp("model", longName) == 0) {
      outValues->modelFilename = optionStringValue;
    } else if (strcmp("positive", longName) == 0) {
      outValues->positiveDirectory = optionStringValue;
    } else if (strcmp("negative", longName) == 0) {
      outValues->negativeDirectory = optionStringValue;
    } else if (strcmp("multisample", longName) == 0) {
      const int optionIntValue = atoi(optionStringValue);
      outValues->doMultisample = optionIntValue;
    } else if (strcmp("time", longName) == 0) {
      const int optionIntValue = atoi(optionStringValue);
      outValues->doTime = optionIntValue;
    } else if (strcmp("threshold", longName) == 0) {
      const float optionFloatValue = atof(optionStringValue);
      outValues->threshold = optionFloatValue;
    } else if (strcmp("layer", longName) == 0) {
      const int optionIntValue = atoi(optionStringValue);
      outValues->layerOffset = -optionIntValue;
    } else if (strcmp("inputdir", longName) == 0) {
      outValues->inputDirectory = optionStringValue;
    } else if (strcmp("outputdir", longName) == 0) {
      outValues->outputDirectory = optionStringValue;
    } else if (strcmp("debug", longName) == 0) {
      const int optionIntValue = atoi(optionStringValue);
      outValues->doDebugLogging = optionIntValue;
    } else {
      assert(false); // Should never get here
    }
  }
}

void print_usage_and_exit(int argc, const char* argv[]) {
  fprintf(stderr, "usage: %s", argv[0]);
  for (int index = 0; index < g_toolOptionsLength; index += 1) {
    SToolOption* toolOption = &g_toolOptions[index];
    fprintf(stderr, " --%s/-%c", toolOption->longName, toolOption->shortName);
    if (toolOption->hasArgument) {
      fprintf(stderr, " <arg>");
    }
  }
  fprintf(stderr, "\n");
  for (int index = 0; index < g_toolOptionsLength; index += 1) {
    SToolOption* toolOption = &g_toolOptions[index];
    fprintf(stderr, "    --%s/-%c: %s",
      toolOption->longName,
      toolOption->shortName,
      toolOption->description);
    if (toolOption->isRequired) {
      fprintf(stderr, " Required.");
    }
    if (toolOption->defaultValue != NULL) {
      fprintf(stderr, " Default is '%s'.", toolOption->defaultValue);
    }
    fprintf(stderr, "\n");
  }
  exit(1);
}

void do_classify_image(void* network, const char* inputFilename, int doMultisample, int layerOffset, float** predictions, int* predictionsLength, char*** predictionsLabels, long* outDuration) {
  void* input = jpcnn_create_image_buffer_from_file(inputFilename);
  if (input == NULL) {
    *predictions = NULL;
    *predictionsLength = 0;
    *predictionsLabels = NULL;
    return;
  }
  int predictionsLabelsLength;
  int actualPredictionsLength;
  struct timeval start;
  gettimeofday(&start, NULL);
  uint32_t flags = 0;
  if (doMultisample) {
    flags = (flags | JPCNN_MULTISAMPLE);
  }
  jpcnn_classify_image(network, input, flags, layerOffset, predictions, &actualPredictionsLength, predictionsLabels, &predictionsLabelsLength);
  struct timeval end;
  gettimeofday(&end, NULL);
  jpcnn_destroy_image_buffer(input);
  long seconds  = end.tv_sec  - start.tv_sec;
  long useconds = end.tv_usec - start.tv_usec;
  *outDuration = ((seconds) * 1000 + useconds/1000.0) + 0.5;
  if (doMultisample) {
    for (int index = 0; index < actualPredictionsLength; index += 1) {
      const float predictionValue = (*predictions)[index];
      const int labelIndex = (index % predictionsLabelsLength);
      const float moduloPredictionValue = (*predictions)[labelIndex];
      (*predictions)[labelIndex] = fmax(predictionValue, moduloPredictionValue);
    }
    *predictionsLength = predictionsLabelsLength;
  } else {
    *predictionsLength = actualPredictionsLength;
  }
}

int has_image_suffix(const char* basename) {
  const size_t basenameLength = strlen(basename);
  const char* knownSuffixes[] = {
    ".jpg", ".jpeg", ".png"
  };
  for (int index = 0; index < STATIC_ARRAY_LEN(knownSuffixes); index += 1) {
    const char* knownSuffix = knownSuffixes[index];
    const size_t knownSuffixLength = strlen(knownSuffix);
    if (basenameLength <= knownSuffixLength) {
      continue;
    }
    const size_t basenameSuffixIndex = (basenameLength - knownSuffixLength);
    const char* basenameSuffix = (basename + basenameSuffixIndex);
    if (strcasecmp(basenameSuffix, knownSuffix) == 0) {
      return 1;
    }
  }
  return 0;
}

void classify_images_in_directory(void* network, const char* directoryName, SToolArgumentValues* argValues, ClassifyImagesFunctionPtr callback, void* callbackCookie) {
  DIR* dir = opendir(directoryName);
  if (dir == NULL) {
    fprintf(stderr, "Couldn't open image directory '%s'\n", directoryName);
    return;
  }

  const size_t directoryNameLength = strlen(directoryName);
  int filesRead = 0;
  long durationTotal = 0;
  struct dirent* dirEntry;
  while ((dirEntry = readdir(dir)) != NULL) {
    const char* basename = dirEntry->d_name;
    if (!has_image_suffix(basename)) {
      continue;
    }
    const size_t basenameLength = strlen(basename);
    const size_t fullPathLength = directoryNameLength + 1 + basenameLength;
    char* fullPath = (char*)(malloc(fullPathLength + 1));
    snprintf(fullPath, (fullPathLength + 1), "%s/%s", directoryName, basename);

    float* predictions;
    int predictionsLength;
    char** predictionsLabels;
    long duration;

    do_classify_image(network,
      fullPath,
      argValues->doMultisample,
      argValues->layerOffset,
      &predictions,
      &predictionsLength,
      &predictionsLabels,
      &duration);

    if (predictions == NULL) {
      free(fullPath);
      continue;
    }

    durationTotal += duration;
    filesRead += 1;

    (*callback)(callbackCookie, predictions, predictionsLength, basename, directoryName, fullPath);

    free(fullPath);
  }
  if (filesRead == 0) {
    fprintf(stderr, "No image files were found in directory '%s'\n", directoryName);
    return;
  }
  if (argValues->doTime) {
    const long averageDuration = (durationTotal / filesRead);
    fprintf(stderr, "Classification took %ld milliseconds on average over %d files\n", averageDuration, filesRead);
  }
}

void training_callback(void* cookie, float* predictions, int predictionsLength, const char* basename, const char* directoryName, const char* fullPath) {
  STrainingCookie* cookieData = (STrainingCookie*)(cookie);
  jpcnn_train(cookieData->trainer, cookieData->label, predictions, predictionsLength);
}

void testing_callback(void* cookie, float* predictions, int predictionsLength, const char* basename, const char* directoryName, const char* fullPath) {
  STestingCookie* cookieData = (STestingCookie*)(cookie);
  void* predictor = cookieData->predictor;
  const float predictedValue = jpcnn_predict(predictor, predictions, predictionsLength);
  const float threshold = cookieData->threshold;
  float predictedLabel;
  if (predictedValue > threshold) {
    predictedLabel = 1.0f;
  } else {
    predictedLabel = 0.0f;
  }
  const float expectedLabel = cookieData->expectedLabel;
  if (expectedLabel == 1.0f) {
    if (predictedLabel == 1.0f) {
      cookieData->truePositiveCount += 1;
    } else {
      cookieData->falseNegativeCount += 1;
    }
  } else {
    if (predictedLabel == 0.0f) {
      cookieData->trueNegativeCount += 1;
    } else {
      cookieData->falsePositiveCount += 1;
    }
  }
  cookieData->total += 1;
}

void prediction_callback(void* cookie, float* predictions, int predictionsLength, const char* basename, const char* directoryName, const char* fullPath) {
  SPredictionCookie* cookieData = (SPredictionCookie*)(cookie);
  void* predictor = cookieData->predictor;
  const float predictedValue = jpcnn_predict(predictor, predictions, predictionsLength);
  const char* outputDirectory = cookieData->outputDirectory;
  const size_t outputDirectoryNameLength = strlen(outputDirectory);
  // Ack, fixed-length strings ahead. We're creating this one based on a fixed
  // precision number and a suffix, and using snprintf, but still icky.
  const char* suffix = strrchr(basename, '.');
  if (!suffix || suffix == basename) {
    suffix = "";
  }
  const size_t outnameMaxLength = 128;
  char outname[outnameMaxLength];
  snprintf(outname, outnameMaxLength, "%1.8f%s", predictedValue, suffix);

  const size_t outnameLength = strlen(outname);
  const size_t outPathLength = outputDirectoryNameLength + 1 + outnameLength;
  char* outPath = (char*)(malloc(outPathLength + 1));
  snprintf(outPath, (outPathLength + 1), "%s/%s", outputDirectory, outname);

  const int symlinkResult = symlink(fullPath, outPath);
  if (symlinkResult != 0) {
    fprintf(stderr, "symlink failed for '%s' -> '%s' with error number %d\n", outPath, fullPath, errno);
  }
  free(outPath);
}

int main(int argc, const char * argv[]) {

  SToolArgumentValues argValues;
  parse_command_line_args(argc, argv, &argValues);

  void* network = jpcnn_create_network(argValues.networkFilename);

  if (argValues.doDebugLogging) {
    jpcnn_print_network(network);
  }

  switch (argValues.mode) {
    case eSingleImage: {
      float* predictions;
      int predictionsLength;
      char** predictionsLabels;
      long duration;

      do_classify_image(network,
        argValues.inputImageFilename,
        argValues.doMultisample,
        argValues.layerOffset,
        &predictions,
        &predictionsLength,
        &predictionsLabels,
        &duration);

      for (int index = 0; index < predictionsLength; index += 1) {
        const float predictionValue = predictions[index];
        if (predictionValue < 0.01f) {
          continue;
        }
        char* label = predictionsLabels[index];
        fprintf(stdout, "%f\t%s\n", predictionValue, label);
      }
      if (argValues.doTime) {
        fprintf(stderr, "Classification took %ld milliseconds\n", duration);
      }
    } break;

    case eLibSvmTrain: {
      void* trainer = jpcnn_create_trainer();
      STrainingCookie cookieData;
      void* cookie = (void*)(&cookieData);
      cookieData.trainer = trainer;

      cookieData.label = 1.0f;
      classify_images_in_directory(network, argValues.positiveDirectory, &argValues, &training_callback, cookie);

      cookieData.label = 0.0f;
      classify_images_in_directory(network, argValues.negativeDirectory, &argValues, &training_callback, cookie);

      void* predictor = jpcnn_create_predictor_from_trainer(trainer);
      const int saveResult = jpcnn_save_predictor(argValues.modelFilename, predictor);
      if (!saveResult) {
        fprintf(stderr, "Couldn't save predictor file to '%s'\n", argValues.modelFilename);
        print_usage_and_exit(argc, argv);
      }
      jpcnn_destroy_trainer(trainer);
      jpcnn_destroy_predictor(predictor);
    } break;

    case eLibSvmTest: {
      void* predictor = jpcnn_load_predictor(argValues.modelFilename);
      if (predictor == NULL) {
        fprintf(stderr, "Couldn't load predictor file from '%s'\n", argValues.modelFilename);
        print_usage_and_exit(argc, argv);
      }
      STestingCookie cookieData;
      void* cookie = (void*)(&cookieData);
      cookieData.predictor = predictor;
      cookieData.threshold = argValues.threshold;
      cookieData.truePositiveCount = 0;
      cookieData.falsePositiveCount = 0;
      cookieData.trueNegativeCount = 0;
      cookieData.falseNegativeCount = 0;
      cookieData.total = 0;

      cookieData.expectedLabel = 1.0f;
      classify_images_in_directory(network, argValues.positiveDirectory, &argValues, testing_callback, cookie);

      cookieData.expectedLabel = 0.0f;
      classify_images_in_directory(network, argValues.negativeDirectory, &argValues, testing_callback, cookie);

      const int truePositives = cookieData.truePositiveCount;
      const int falsePositives = cookieData.falsePositiveCount;
      const int trueNegatives = cookieData.trueNegativeCount;
      const int falseNegatives = cookieData.falseNegativeCount;
      const int positivesTotal = (truePositives + falseNegatives);
      const int negativesTotal = (trueNegatives + falsePositives);

      fprintf(stderr, "True positives = %.2f%% (%d/%d)\n", ((truePositives * 100.0f) / positivesTotal), truePositives, positivesTotal);
      fprintf(stderr, "True negatives = %.2f%% (%d/%d)\n", ((trueNegatives * 100.0f) / negativesTotal), trueNegatives, negativesTotal);
      fprintf(stderr, "False positives = %.2f%% (%d/%d)\n", ((falsePositives * 100.0f) / negativesTotal), falsePositives, negativesTotal);
      fprintf(stderr, "False negatives = %.2f%% (%d/%d)\n", ((falseNegatives * 100.0f) / positivesTotal), falseNegatives, positivesTotal);

      jpcnn_destroy_predictor(predictor);
    } break;

    case eLibSvmPredict: {
      void* predictor = jpcnn_load_predictor(argValues.modelFilename);
      if (predictor == NULL) {
        fprintf(stderr, "Couldn't load predictor file from '%s'\n", argValues.modelFilename);
        print_usage_and_exit(argc, argv);
      }
      SPredictionCookie cookieData;
      void* cookie = (void*)(&cookieData);
      cookieData.outputDirectory = argValues.outputDirectory;
      cookieData.predictor = predictor;
      classify_images_in_directory(network, argValues.inputDirectory, &argValues, prediction_callback, cookie);
      jpcnn_destroy_predictor(predictor);
    } break;

    default: {
      assert(false); // Should never get here
    } break;
  }

  jpcnn_destroy_network(network);

  return 0;
}

