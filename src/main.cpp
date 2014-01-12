//
//  main.cpp
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#include <iostream>

#include "buffer.h"

#include "prepareinput.h"

int main(int argc, const char * argv[])
{
  if (argc < 2) {
    fprintf(stderr, "Usage: jpcnn <input file>\n");
    return 1;
  }

  Buffer* input = buffer_from_image_file((char*)(argv[1]));

  fprintf(stderr, "input=%s\n", input->debugString());

  const char* expectedFileName = "data/lena_blobs/000_input_data.blob";
  Buffer* expectedInput = buffer_from_dump_file(expectedFileName);
  expectedInput->setName("expectedInput");

  const char* dataMeanFileName = "data/data_mean.blob";
  Buffer* dataMean = buffer_from_dump_file(dataMeanFileName);

  PrepareInput prepareInput(dataMean, true);

  Buffer* rescaledInput = prepareInput.run(input);

  expectedInput->saveDebugImage();
  rescaledInput->saveDebugImage();

  const bool isRescaledEqual = buffer_are_all_close(expectedInput, rescaledInput);
  if (!isRescaledEqual) {
    fprintf(stderr, "Rescaled buffers aren't equal.\n");
  } else {
    fprintf(stderr, "Rescaled buffers are equal.\n");
  }

  return 0;
}

