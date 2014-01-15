//
//  main.cpp
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#include <stdio.h>
#include <sys/time.h>

#include "buffer.h"
#include "prepareinput.h"
#include "graph.h"

int main(int argc, const char * argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: jpcnn <input image>\n");
    return 1;
  }

  Graph* graph = new_graph_from_file("data/graph.btag");

  Buffer* input = buffer_from_image_file((char*)(argv[1]));

  PrepareInput prepareInput(graph->_dataMean, true);

  Buffer* rescaledInput = prepareInput.run(input);

  struct timeval start;
  gettimeofday(&start, NULL);
  Buffer* predictions = graph->run(rescaledInput);
  struct timeval end;
  gettimeofday(&end, NULL);

  const Dimensions predictionsDims = predictions->_dims;
  const int imageCount = predictionsDims[0];
  const int labelsCount = predictionsDims[1];

  for (int imageIndex = 0; imageIndex < imageCount; imageIndex += 1) {
    for (int labelIndex = 0; labelIndex < labelsCount; labelIndex += 1) {
      const int predictionIndex = predictionsDims.offset(imageIndex, labelIndex);
      const jpfloat_t labelValue = predictions->_data[predictionIndex];
      char* labelName = graph->_labelNames[labelIndex];
      fprintf(stdout, "%f\t%s\n", labelValue, labelName);
    }
  }

  long seconds  = end.tv_sec  - start.tv_sec;
  long useconds = end.tv_usec - start.tv_usec;

  long mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;

  fprintf(stderr, "Classification took %ld milliseconds\n", mtime);

  return 0;
}

