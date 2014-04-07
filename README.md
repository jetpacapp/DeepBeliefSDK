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

 - [Getting Started](#getting-started)
 - [A Simple Example](#a-simple-example)
 - [API Reference](#api-reference)
 - [More Information](#more-information)
 - [License](#license)
 - [Credits](#credits)

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

## A Simple Example

To use the library in your own application, you need to add the DeepBelief.framework bundle to the Link Binary with Libraries build phase in your XCode project settings, and add `#import <DeepBelief/DeepBelief.h>` to the top of the file you want to use the code in.

You should then be able to use code like this to classify a single image that you've included as a resource in your bundle. The code assumes it's called 'dog.jpg', but you should change it to match the name of your file.

```objectivec
  NSString* networkPath = [[NSBundle mainBundle] pathForResource:@"jetpac" ofType:@"ntwk"];
  if (networkPath == NULL) {
    fprintf(stderr, "Couldn't find the neural network parameters file - did you add it as a resource to your application?\n");
    assert(false);
  }
  network = jpcnn_create_network([networkPath UTF8String]);
  assert(network != NULL);

  NSString* imagePath = [[NSBundle mainBundle] pathForResource:@"dog" ofType:@"jpg"];
  void* inputImage = jpcnn_create_image_buffer_from_file([imagePath UTF8String]);

  float* predictions;
  int predictionsLength;
  char** predictionsLabels;
  int predictionsLabelsLength;
  jpcnn_classify_image(network, inputImage, 0, 0, &predictions, &predictionsLength, &predictionsLabels, &predictionsLabelsLength);

  jpcnn_destroy_image_buffer(inputImage);

  for (int index = 0; index < predictionsLength; index += 1) {
    const float predictionValue = predictions[index];
    char* label = predictionsLabels[index % predictionsLabelsLength];
    NSString* predictionLine = [NSString stringWithFormat: @"%s - %0.2f\n", label, predictionValue];
    NSLog(@"%@", predictionLine);
  }
  
  jpcnn_destroy_network(network);
```

## API Reference

Because we reuse the same code across a lot of different platforms, we use a
plain-old C interface to our library. All of the handles to different objects are
opaque pointers, and you have to explictly call the `*_destroy_*` function on any
handles that have been returned from `*_create_*` calls if you want to avoid memory
leaks. Input images are created from raw arrays of 8-bit RGB data, you can see how 
to build those from iOS types by searching for `jpcnn_create_image_buffer()` in the 
sample code.

The API is broken up into two sections. The first gives you access to a pre-trained neural network, currently the example jetpac.ntwk file is the only one available. 
This has been trained on 1,000 Imagenet categories, and the output will give you a decent general idea of what's in an image.

The second section lets you replace the highest layer of the neural network with your own classification step. 
This means you can use it to recognize the objects you care about more accurately.

### Pre-trained calls

 - [jpcnn_create_network](#jpcnn_create_network)
 - [jpcnn_destroy_network](#jpcnn_destroy_network)
 - [jpcnn_create_image_buffer_from_file](#jpcnn_create_image_buffer_from_file)
 - [jpcnn_create_image_buffer_from_uint8_data](#jpcnn_create_image_buffer_from_uint8_data)
 - [jpcnn_destroy_image_buffer](#jpcnn_destroy_image_buffer)
 - [jpcnn_classify_image](#jpcnn_classify_image)
 - [jpcnn_print_network](#jpcnn_print_network)

### Custom training calls

 - [jpcnn_create_trainer](#jpcnn_create_trainer)
 - [jpcnn_destroy_trainer](#jpcnn_destroy_trainer)
 - [jpcnn_train](#jpcnn_train)
 - [jpcnn_create_predictor_from_trainer](#jpcnn_create_predictor_from_trainer)
 - [jpcnn_destroy_predictor](#jpcnn_destroy_predictor)
 - [jpcnn_load_predictor](#jpcnn_load_predictor)
 - [jpcnn_print_predictor](#jpcnn_print_predictor)
 - [jpcnn_predict](#jpcnn_predict)

### jpcnn_create_network

`void* jpcnn_create_network(const char* filename)`

This takes the filename of the network parameter file as an input, and builds a 
neural network stack based on that definition. Right now the only available file
is the 1,000 category jetpac.ntwk, built here at Jetpac based on the
approach used by Krizhevsky to win the Imagenet 2012 competition.

You'll need to make sure you include this 60MB file in the 'Copy Files' build phase
of your application, and then call something like this to get the actual path:

```
NSString* networkPath = [[NSBundle mainBundle] pathForResource:@"jetpac" ofType:@"ntwk"];
network = jpcnn_create_network([networkPath UTF8String]);
```

### jpcnn_destroy_network

`void jpcnn_destroy_network(void* networkHandle)`

Once you're finished with the neural network, call this to destroy it and free
up the memory it used.

### jpcnn_create_image_buffer_from_file

`void* jpcnn_create_image_buffer_from_file(const char* filename)`

Takes a filename (see above for how to get one from your bundle) and creates an
image object that you can run the classification process on. It can load PNGS and
JPEGS.

### jpcnn_create_image_buffer_from_uint8_data

`void* jpcnn_create_image_buffer_from_uint8_data(unsigned char* pixelData, int width, int height, int channels, int rowBytes, int reverseOrder, int doRotate)`

If you already have data in memory, you can use this function to copy it into an
image object that you can then classify. It's useful if you're doing video capture,
as the sample code does.

### jpcnn_destroy_image_buffer

`void jpcnn_destroy_image_buffer(void* imageHandle)`

Once you're done classifying an image, call this to free up the memory it used.

### jpcnn_classify_image

`void jpcnn_classify_image(void* networkHandle, void* inputHandle, unsigned int flags, int layerOffset, float** outPredictionsValues, int* outPredictionsLength, char*** outPredictionsNames, int* outPredictionsNamesLength)`

This is how you actually get tags for an image. It takes in a neural network and an
image, and returns an array of floats. Each float is a predicted value for an
imagenet label, between 0 and 1, where higher numbers are more confident predictions.

The three outputs are:

 - outPredictionsValues is a pointer to the array of predictions.
 - outPredictionsLength holds the length of the predictions array.
 - outPredictionsNames is an array of C strings representing imagenet labels, each corresponding to the prediction value at the same index in outPredictionsValues.
 - outPredictionsNamesLength is the number of name strings in the label array. In the simple case this is the same as the number of predictions, but in different modes this can get more complicated! See below for details.
 
In the simple case you can leave the flags and layerOffset arguments as zero, and you'll get an array of prediction values out. Pick the highest (possibly with a threshold like 0.1 to avoid shaky ones), and you can use that as a simple tag for the image.

There are several optional arguments you can use to improve your results though.

#### layerOffset

The final output of the neural network represents the high-level categories that it's been trained on, but often you'll want to work with other types of objects.
The good news is that it's possible to take the results from layers that are just before the final one, and use those as inputs to simple statistical algorithms to recognize entirely new kinds of things.
[This paper on Decaf](http://arxiv.org/abs/1310.1531) does a good job of describing the approach, but the short version is that those high-level layers can be seen as adjectives that help the output layer make its final choice between categories, and those same adjectives turn out to be useful for choosing between a lot of other categories it hasn't been trained on too.
For example, there might be some signals that correlate with 'spottiness' and 'furriness', which would be useful for picking out leopards, even if they were originally learned from pictures of dalmatians.

The `layerOffset` argument lets you control which layer you're sampling, as a negative offset from the start of the network. 
Try setting it to `-2`, and you should get an array of 4096 floats in outPredictionsValues, though since these are no longer representing Imagenet labels the names array will no longer be valid. 
You can then feed those values into a training system like libSVM to help you distinguish between the kinds of objects you care about.

#### flags

The image recognition algorithm always crops the input image to the biggest square that fits within its bounds, resamples that area to 256x256 pixels and then takes a slightly smaller 224x224 sample square from somewhere within that main square.
The flags argument controls how that 224-pixel sample square is positioned within the larger one. If it's left as zero, then it's centered with a 16 pixel margin at all edges. 
The sample code uses `JPCNN_RANDOM_SAMPLE` to jitter the origin of the 224 square randomly within the bounds each call, since this, combined with smoothing of the results over time, helps ensure that the identification of tags is robust to slight position changes.
The `JPCNN_MULTISAMPLE` flag takes ten different sample positions within the image and runs them all through the classification pipeline simultaneously. This is a costly operation, so it doesn't tend to be practical on low-processing-power platforms like the iPhone.

### jpcnn_print_network

`void jpcnn_print_network(void* networkHandle)`

This is a debug logging call that prints information about a loaded neural network.

### jpcnn_create_trainer

`void* jpcnn_create_trainer()`

Returns a handle to a trainer object that you can feed training examples into to build your own custom prediction model.

### jpcnn_destroy_trainer

`void jpcnn_destroy_trainer(void* trainerHandle)`

Disposes of the memory used by the trainer object and destroys it.

### jpcnn_train

`void jpcnn_train(void* trainerHandle, float expectedLabel, float* predictions, int predictionsLength)`

To create your own custom prediction model, you need to train it using 'positive' examples of images containing the object you care about, and 'negative' examples of images that don't.
Once you've created a trainer object, you can call this with the neural network results for each positive or negative image, and with an expectedLabel of '0.0' for negatives and '1.0' for positives.
Picking the exact number of each you'll need is more of an art than a science, since it depends on how easy your object is to recognize and how cluttered your environment is, but I've had decent results with as few as a hundred of each.
You can use the output of any layer of the neural network, but I've found using the penultimate one works well. I discuss how to do this above in the [layerOffset](#layeroffset) section.
To see how this works in practice, try out the [LearningExample](#learningexample) sample code for yourself.

### jpcnn_create_predictor_from_trainer

`void* jpcnn_create_predictor_from_trainer(void* trainerHandle)`

Once you've passed in all your positive and negative examples to [jpcnn_train](#jpcnn_train), you can call this to build a predictor model from them all. 
Under the hood, it's using libSVM to create a support vector machine model based on the examples.

### jpcnn_destroy_predictor

`void jpcnn_destroy_predictor(void* predictorHandle)`

Deallocates any memory used by the predictor model, call this once you're finished with it.

### jpcnn_load_predictor

`void* jpcnn_load_predictor(const char* filename)`

Loads a predictor you've already created from a libSVM-format text file. 
Since you can't save files on iOS devices, the only way to create this file in the first place is to call [jpcnn_print_predictor](#jpcnn_print_predictor) once you've created a predictor, and then copy and paste the results from the developer console into a file, and then add it to your app's resources. 
The [SavedModelExample](#savedmodelExample) sample code shows how to use this call.

### jpcnn_print_predictor

`void jpcnn_print_predictor(void* predictorHandle)`

Outputs the parameters that define a custom predictor to stderr (and hence the developer console in XCode). You'll need to copy and paste this into your own text file to subsequently reload the predictor.

### jpcnn_predict

`float jpcnn_predict(void* predictorHandle, float* predictions, int predictionsLength)`

Given the output from a pre-trained neural network, and a custom prediction model, returns a value estimating the probability that the image contains the object it has been trained against.

## More Information

Join the [Deep Belief Developers email list](https://groups.google.com/group/deep-belief-developers) to find out more about the practical details of implementing deep learning.

## License

The binary framework and network parameter file are under the BSD three-clause
license, included in this folder as LICENSE.

## Credits

Big thanks go to:

 - [Daniel Nouri](http://danielnouri.org/) for his invaluable help on CNNs.
 - [Alex Krizhevsky, Ilya Sutskever, and Geoffrey Hinton](http://www.cs.toronto.edu/~hinton/absps/imagenet.pdf) for their ground-breaking work on ConvNet.
 - [Yann LeCun](https://plus.google.com/+YannLeCunPhD/posts) for his pioneering research and continuing support of the field.
 - The Berkeley Vision and Learning Center for their work on [lightweight custom training of CNNs](https://github.com/BVLC/caffe).
 - My colleagues [Cathrine](https://twitter.com/lindblomsten), [Dave](https://twitter.com/davefearon), [Julian](https://twitter.com/juliangreensf), and [Sophia](https://twitter.com/spara) at [Jetpac](https://www.jetpac.com/) for all their help.
 
[Pete Warden](https://twitter.com/petewarden)
