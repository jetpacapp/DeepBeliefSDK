#include "opencv2/highgui.hpp"
#include "opencv2/core/utility.hpp"

#include <string>

#include "deepbeliefopencv.h"

int main( int argc, const char** argv ) {

  std::string imageName;
  if (argc > 1) {
    imageName.assign(argv[1]);
  } else {
    imageName = "lena.png";
  }

  cv::Mat cvImage = cv::imread(imageName);

  DeepBelief::Image* deepBeliefImage = new DeepBelief::Image(cvImage);

  std::string networkName = "jetpac.ntwk";
  DeepBelief::Network* network = new DeepBelief::Network(networkName);

  DeepBelief::ClassificationResult result = network->classifyImage(*deepBeliefImage);

  result.print();

  delete deepBeliefImage;
  delete network;

  return 0;
}