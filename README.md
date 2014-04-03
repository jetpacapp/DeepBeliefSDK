DeepBeliefSDK
=============

The SDK for [Jetpac's](https://www.jetpac.com) iOS Deep Belief image recognition framework.

This is a compiled framework implementing the convolutional neural network 
architecture [described by Alex Krizhevsky, Ilya Sutskever, and Geoffrey Hinton](http://www.cs.toronto.edu/~hinton/absps/imagenet.pdf).
The processing code has been highly optimized to run within the memory and 
processing constraints of iOS devices, and can analyze an image in under 300ms on 
an iPhone 5S. We're releasing this framework because we're excited by the power of
this approach for general image recognition, especially when it can run locally on
low-power devices. It gives your iPhone the ability to see, and I can't wait to see
what applications that lets you build.

## Getting Started

You'll need the usual tools required for developing iOS applications - XCode 5, an
OS X machine and a modern iOS device (it's been tested as far back as the original
iPhone 4). Open up the DeepBeliefExample/DeepBeliefExample.xcodeproj, build and run.

You should see some warnings (the example is based on Apple sample code which has
some anachronisms in it unfortunately), then once it's running a live camera stream
should be visible on your phone. Move it to look closely at your keyboard, and some
tags should start appearing in the top left of the screen. These should include
things that look like keyboards, including calculators, remote controls, and even
typewriters!

You should experiment with other objects like coffee cups, doors, televisions, and
even dogs if you have any handy! The results will not be human quality, but the 
important part is that they're capturing meaningful attributes of the images.
Understanding images with no context is extremely hard, and while this approach
is a massive step forward compared to the previous state of the art, you'll still
need to adapt it to the domain you're working in to get the best results in a real
application.

## API Reference

Because we reuse the same code across a lot of different platforms, we use a
plain-old C interface to our library. All of the handles to different objects are
opaque pointers, and you have to explictly call the `*_destroy_*` function on any
handles that have been returned from `*_create_*` calls. Input images are created
from raw arrays of 8-bit RGB data, you can see how to build those from iOS types
by searching for `jpcnn_create_image_buffer()` in the sample code.

`void* jpcnn_create_network(const char* filename)`

`void jpcnn_destroy_network(void* networkHandle)`

`void* jpcnn_create_image_buffer_from_file(const char* filename)`

`void jpcnn_destroy_image_buffer(void* imageHandle)`

`void* jpcnn_create_image_buffer_from_uint8_data(unsigned char* pixelData, int width, int height, int channels, int rowBytes, int reverseOrder, int doRotate)`

`void jpcnn_classify_image(void* networkHandle, void* inputHandle, unsigned int flags, int layerOffset, float** outPredictionsValues, int* outPredictionsLength, char*** outPredictionsNames, int* outPredictionsNamesLength)`

`void jpcnn_print_network(void* networkHandle)`

## More Information


## Licensing

The binary framework and network parameter file are under the BSD three-clause
license, included in this folder as LICENSE.


