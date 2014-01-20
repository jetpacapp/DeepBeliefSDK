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

#include "libjpcnn.h"

#define STATIC_ARRAY_LEN(x) (sizeof(x) / sizeof(x[0]))

enum EToolMode {eSingleImage, eLibSvmTrain};
typedef struct SToolArgumentValuesStruct {
  const char* networkFilename;
  const char* inputImageFilename;
  const char* outputFilename;
  const char* positiveDirectory;
  const char* negativeDirectory;
  int doMultisample;
  EToolMode mode;
  int doTime;
} SToolArgumentValues;

typedef struct SToolOptionStruct {
  const char* longName;
  const char shortName;
  int isRequired;
  int hasArgument;
  const char* defaultValue;
  const char* description;
} SToolOption;

static SToolOption g_toolOptions[] = {
  {"network", 'n', 1, 1, NULL, "The path to the neural network parameter file."},
  {"mode", 'm', 1, 1, NULL, "Which operation to perform. Can be 'single'/'s' to analyze one image or 'train'/'t' to produce a suitable training file for libSVM from folders of positive and negative images."},
  {"input", 'i', 0, 1, NULL, "The path to a single input image."},
  {"positive", 'p', 0, 1, NULL, "The path to a folder of positive images."},
  {"negative", 'e', 0, 1, NULL, "The path to a folder of negative images."},
  {"multisample", 's', 0, 0, "0", "Whether to use a higher-quality but slower strategy of running the detection against ten different transforms of the image."},
  {"time", 't', 0, 0, "0", "Whether to print a the time taken by the classification algorithm to stderr."},
  {"output", 'o', 0, 1, NULL, "The file to write the output text to (defaults to stdout if not set)."},
};
const int g_toolOptionsLength = STATIC_ARRAY_LEN(g_toolOptions);

static void parse_command_line_args(int argc, const char* argv[], SToolArgumentValues* outValues);
static void print_usage_and_exit(int argc, const char* argv[]);
static void do_classify_image(void* network, const char* inputFilename, int doMultisample, float** predictions, int* predictionsLength, char*** predictionsLabels, long* outDuration);
static int has_image_suffix(const char* basename);
static void classify_images_in_directory(void* network, const char* directoryName, const char* outputPrefix, FILE* outputFile, SToolArgumentValues* argValues);

void parse_command_line_args(int argc, const char* argv[], SToolArgumentValues* outValues) {

  struct option getOptOptions[STATIC_ARRAY_LEN(g_toolOptions) + 1];
  for (int index = 0; index < g_toolOptionsLength; index += 1) {
    SToolOption* toolOption = &g_toolOptions[index];
    struct option* getOptOption = &getOptOptions[index];
    getOptOption->name = toolOption->longName;
    if (!toolOption->hasArgument) {
      getOptOption->has_arg = no_argument;
    } else {
      if (toolOption->defaultValue == NULL) {
        getOptOption->has_arg = required_argument;
      } else {
        getOptOption->has_arg = optional_argument;
      }
    }
    getOptOption->flag = NULL;
    getOptOption->val = index;
  }
  struct option* lastGetOptOption = &getOptOptions[g_toolOptionsLength];
  memset(lastGetOptOption, 0, sizeof(*lastGetOptOption));

  char shortGetOptOptions[((STATIC_ARRAY_LEN(g_toolOptions) * 2) + 1)];
  int shortOutputIndex = 0;
  for (int index = 0; index < g_toolOptionsLength; index += 1) {
    SToolOption* toolOption = &g_toolOptions[index];
    shortGetOptOptions[shortOutputIndex] = toolOption->shortName;
    shortOutputIndex += 1;
    if (toolOption->hasArgument) {
      shortGetOptOptions[shortOutputIndex] = ':';
      shortOutputIndex += 1;
    }
  }
  shortGetOptOptions[shortOutputIndex] = '\0';

  const char* optionStringValues[g_toolOptionsLength];
  for (int index = 0; index < g_toolOptionsLength; index += 1) {
    SToolOption* toolOption = &g_toolOptions[index];
    optionStringValues[index] = toolOption->defaultValue;
  }

  while (1) {
    int optionIndex = -1;
    const int getOptResult = getopt_long(argc, (char* const*)(argv),  shortGetOptOptions,
      getOptOptions, &optionIndex);
    if (getOptResult == -1) {
      break;
    }
    if (optionIndex == -1) {
      bool wasFound = false;
      for (int index = 0; index < g_toolOptionsLength; index += 1) {
        SToolOption* toolOption = &g_toolOptions[index];
        if (toolOption->shortName == getOptResult) {
          optionIndex = index;
          wasFound = true;
          break;
        }
      }
      if (!wasFound) {
        print_usage_and_exit(argc, argv);
      }
    }
    SToolOption* toolOption = &g_toolOptions[optionIndex];
    if (toolOption->hasArgument) {
      optionStringValues[optionIndex] = optarg;
    } else {
      optionStringValues[optionIndex] = "1";
    }
  }

  for (int index = 0; index < g_toolOptionsLength; index += 1) {
    const char* optionStringValue = optionStringValues[index];
    SToolOption* toolOption = &g_toolOptions[index];
    if (toolOption->isRequired &&(optionStringValue == NULL)) {
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
      } else {
        fprintf(stderr, "Unknown argument to --mode: '%s'\n", optionStringValue);
        print_usage_and_exit(argc, argv);
      }
    } else if (strcmp("input", longName) == 0) {
      outValues->inputImageFilename = optionStringValue;
    } else if (strcmp("output", longName) == 0) {
      outValues->outputFilename = optionStringValue;
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

void do_classify_image(void* network, const char* inputFilename, int doMultisample, float** predictions, int* predictionsLength, char*** predictionsLabels, long* outDuration) {
  void* input = jpcnn_create_image_buffer_from_file(inputFilename);
  int predictionsLabelsLength;
  int actualPredictionsLength;
  struct timeval start;
  gettimeofday(&start, NULL);
  jpcnn_classify_image(network, input, doMultisample, predictions, &actualPredictionsLength, predictionsLabels, &predictionsLabelsLength);
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

void classify_images_in_directory(void* network, const char* directoryName, const char* outputPrefix, FILE* outputFile, SToolArgumentValues* argValues) {
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
      &predictions,
      &predictionsLength,
      &predictionsLabels,
      &duration);

    durationTotal += duration;
    filesRead += 1;

    fprintf(outputFile, "%s", outputPrefix);
    for (int index = 0; index < predictionsLength; index += 1) {
      const float predictionValue = predictions[index];
      fprintf(outputFile, " %d:%f", (index + 1), predictionValue);
    }
    fprintf(outputFile, "\n");
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

int main(int argc, const char * argv[]) {

  SToolArgumentValues argValues;
  parse_command_line_args(argc, argv, &argValues);

  void* network = jpcnn_create_network(argValues.networkFilename);

  FILE* outputFile;
  if (argValues.outputFilename) {
    outputFile = fopen((char*)(argValues.outputFilename), "wb");
    if (outputFile == NULL) {
      fprintf(stderr, "Couldn't open output file '%s'\n", argValues.outputFilename);
      print_usage_and_exit(argc, argv);
    }
  } else {
    outputFile = stdout;
  }

  if (argValues.mode == eSingleImage) {
    float* predictions;
    int predictionsLength;
    char** predictionsLabels;
    long duration;

    do_classify_image(network,
      argValues.inputImageFilename,
      argValues.doMultisample,
      &predictions,
      &predictionsLength,
      &predictionsLabels,
      &duration);

    for (int index = 0; index < predictionsLength; index += 1) {
      const float predictionValue = predictions[index];
      char* label = predictionsLabels[index];
      fprintf(stdout, "%f\t%s\n", predictionValue, label);
    }
    if (argValues.doTime) {
      fprintf(stderr, "Classification took %ld milliseconds\n", duration);
    }
  } else if (argValues.mode == eLibSvmTrain) {
    classify_images_in_directory(network, argValues.positiveDirectory, "1", outputFile, &argValues);
    classify_images_in_directory(network, argValues.negativeDirectory, "0", outputFile, &argValues);
  } else {
    assert(false); // Should never get here
  }

  if (argValues.outputFilename) {
    fclose(outputFile);
  }

  jpcnn_destroy_network(network);

  return 0;
}

