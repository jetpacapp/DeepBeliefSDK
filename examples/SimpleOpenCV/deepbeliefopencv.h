//
//  deepbeliefopencv.h
//  DeepBeliefSDK
//
//  Created by Peter Warden on 4/28/14.
//  Copyright (c) 2014 Jetpac, Inc. Freely reusable sample code.
//
//  These classes show how to wrap DeepBeliefSDK objects in C++,
//  and provide a simple interface to OpenCV Mat images.

#ifndef INCLUDED_DEEPBELIEFOPENCV_H
#define INCLUDED_DEEPBELIEFOPENCV_H

#include <string>

#include "opencv2/core/utility.hpp"

namespace DeepBelief {

  class Image {
  public:
    Image(const std::string& filename);
    Image(const cv::Mat& image);
    ~Image();

    void* imageHandle;
  };

  class ClassificationResult {
  public:

    void print();

    float* predictions;
    int predictionsLength;
    char** predictionsLabels;
    int predictionsLabelsLength;
  };

  class Network {
  public:
    Network(const std::string& filename);
    ~Network();

    ClassificationResult classifyImage(Image& image);

    void* networkHandle;
  };

}

#endif // INCLUDED_DEEPBELIEFOPENCV_H