//
//  main.cpp
//  jpcnn
//
//  Created by Peter Warden on 1/9/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#include <iostream>

#include "buffer.h"

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

  const char* testFileName = "data/lena_blobs/001_input_conv1.blob";
  Buffer* testInput = buffer_from_dump_file(testFileName);

  testInput->_data[0] += 0.00001f;

  const bool isEqual = buffer_are_all_close(expectedInput, testInput);

  if (!isEqual) {
    fprintf(stderr, "Buffers aren't equal.\n");
  } else {
    fprintf(stderr, "Buffers are equal.\n");
  }

  return 0;
}

