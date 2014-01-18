//
//  main.cpp
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#include <stdio.h>
#include <sys/time.h>

#include "libjpcnn.h"

int main(int argc, const char * argv[]) {
  if (argc < 3) {
    fprintf(stderr, "Usage: jpcnn <network file> <input image>\n");
    return 1;
  }

  void* network = jpcnn_create_network(argv[1]);
  void* input = jpcnn_create_image_buffer_from_file(argv[2]);

  float* predictions;
  int predictionsLength;
  char** predictionsLabels;
  int predictionsLabelsLength;
  struct timeval start;
  gettimeofday(&start, NULL);
  jpcnn_classify_image(network, input, 0, &predictions, &predictionsLength, &predictionsLabels, &predictionsLabelsLength);
  struct timeval end;
  gettimeofday(&end, NULL);
  jpcnn_destroy_image_buffer(input);

   for (int index = 0; index < predictionsLength; index += 1) {
    const float predictionValue = predictions[index];
    char* label = predictionsLabels[index % predictionsLabelsLength];
    fprintf(stdout, "%f\t%s\n", predictionValue, label);
  }

  long seconds  = end.tv_sec  - start.tv_sec;
  long useconds = end.tv_usec - start.tv_usec;

  long mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;

  fprintf(stderr, "Classification took %ld milliseconds\n", mtime);

  jpcnn_destroy_network(network);

  return 0;
}

