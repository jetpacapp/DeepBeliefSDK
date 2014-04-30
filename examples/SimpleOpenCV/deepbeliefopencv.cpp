//
//  deepbeliefopencv.cpp
//  DeepBeliefSDK
//
//  Created by Peter Warden on 4/28/14.
//  Copyright (c) 2014 Jetpac, Inc. Freely reusable sample code.
//
//  These classes show how to wrap DeepBeliefSDK objects in C++,
//  and provide a simple interface to OpenCV Mat images.

#include "deepbeliefopencv.h"

#include <cctype>
#include <iostream>
#include <iterator>

#include <stdio.h>
#include <assert.h>

#include "opencv2/core/utility.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgproc/types_c.h"
#include "opencv2/highgui/highgui_c.h"

#include "libjpcnn.h"

using namespace DeepBelief;

Image::Image(const std::string& filename) {
  imageHandle = jpcnn_create_image_buffer_from_file(filename.c_str());
  assert(imageHandle != NULL);
}

Image::Image(const cv::Mat& image) {
  const cv::Size size = image.size();
  const int width = size.width;
  const int height = size.height;
  const int pixelCount = (width * height);
  const int bytesPerPixel = 3;
  const int byteCount = (pixelCount * bytesPerPixel);

  // OpenCV images are BGR, we need RGB, so do a conversion to a temporary image
  cv::Mat rgbImage;
  cv::cvtColor(image, rgbImage, CV_BGR2RGB);
  uint8_t* rgbPixels = (uint8_t*)rgbImage.data;

  imageHandle = jpcnn_create_image_buffer_from_uint8_data(rgbPixels, width, height, 3, (3 * width), 0, 0);
}

Image::~Image() {
  jpcnn_destroy_image_buffer(imageHandle);
}

Network::Network(const std::string& filename) {
  networkHandle = jpcnn_create_network(filename.c_str());
  assert(networkHandle != NULL);
}

Network::~Network() {
  jpcnn_destroy_network(networkHandle);
}

void ClassificationResult::print() {
  for (int index = 0; index < predictionsLength; index += 1) {
    const float predictionValue = predictions[index];
    if (predictionValue < 0.01f) {
      continue;
    }
    char* label = predictionsLabels[index];
    fprintf(stdout, "%f\t%s\n", predictionValue, label);
  }
}

ClassificationResult Network::classifyImage(Image& image) {
  ClassificationResult result;
  jpcnn_classify_image(
    networkHandle,
    image.imageHandle,
    0,
    0,
    &result.predictions,
    &result.predictionsLength,
    &result.predictionsLabels,
    &result.predictionsLabelsLength);
  return result;
}
