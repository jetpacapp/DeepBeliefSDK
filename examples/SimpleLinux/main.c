//
//  main.c
//  DeepBeliefSDK
//
//  Created by Peter Warden on 4/28/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#include <stdio.h>

#include <libjpcnn.h>

#define NETWORK_FILE_NAME "jetpac.ntwk"

int main(int argc, const char * argv[]) {

  const char* imageFileName;
  void* imageHandle;
  void* networkHandle;

  float* predictions;
  int predictionsLength;
  char** predictionsLabels;
  int predictionsLabelsLength;
  int index;

  if (argc < 2) {
    imageFileName = "lena.png";
  } else {
    imageFileName = argv[1];
  }

  const char* networkFileName;
  if (argc < 3) {
    networkFileName = NETWORK_FILE_NAME;
  } else {
    networkFileName = argv[2];
  }

  networkHandle = jpcnn_create_network(networkFileName);
  if (networkHandle == NULL) {
    fprintf(stderr, "DeepBeliefSDK: Couldn't load network file '%s'\n", networkFileName);
    return 1;
  }

  imageHandle = jpcnn_create_image_buffer_from_file(imageFileName);
  if (imageHandle == NULL) {
    fprintf(stderr, "DeepBeliefSDK: Couldn't load image file '%s'\n", imageFileName);
    return 1;
  }

  jpcnn_classify_image(networkHandle, imageHandle, 0, 0, &predictions, &predictionsLength, &predictionsLabels, &predictionsLabelsLength);

  jpcnn_destroy_image_buffer(imageHandle);

  for (index = 0; index < predictionsLength; index += 1) {
    float predictionValue;
    char* label;
    predictionValue = predictions[index];
    if (predictionValue < 0.01f) {
      continue;
    }
    label = predictionsLabels[index];
    fprintf(stdout, "%f\t%s\n", predictionValue, label);
  }

  jpcnn_destroy_network(networkHandle);

  return 0;
}
