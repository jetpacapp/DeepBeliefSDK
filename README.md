DeepBeliefSDK
=============

The SDK for [Jetpac's](https://www.jetpac.com) iOS, Android, Linux, and OS X Deep Belief image recognition framework.

![](http://petewarden.files.wordpress.com/2014/04/learningshot61.png)

This is a compiled framework implementing the convolutional neural network 
architecture [described by Alex Krizhevsky, Ilya Sutskever, and Geoffrey Hinton](http://www.cs.toronto.edu/~hinton/absps/imagenet.pdf).
The processing code has been highly optimized to run within the memory and 
processing constraints of modern mobile devices, and can analyze an image in under 300ms on
an iPhone 5S. It's also easy to use together with OpenCV.

We're releasing this framework because we're excited by the power of
this approach for general image recognition, especially when it can run locally on
low-power devices. It gives your phone the ability to see, and I can't wait to see
what applications that helps you build.

### Getting started

 - [iOS](#getting-started-on-ios)
 - [Android](#getting-started-on-android)
 - [Linux](#getting-started-on-linux)
 - [OS X](#getting-started-on-os-x)
 - [Raspberry Pi](#getting-started-on-a-raspberry-pi)

### Adding to an existing application

 - [iOS](#adding-to-an-existing-ios-application)
 - [Android](#adding-to-an-existing-android-application)
 - [Linux](#adding-to-an-existing-linux-application)
 - [OS X](#adding-to-an-existing-os-x-application)
 - [Using with OpenCV](#using-with-opencv)

### Documentation

 - [Examples](#examples)
 - [Networks](#networks)
 - [API Reference](#api-reference)
 - [FAQ](#faq)
 - [More Information](#more-information)
 - [License](#license)
 - [Credits](#credits)

## Getting Started on iOS

You'll need the usual tools required for developing iOS applications - XCode 5, an
OS X machine and a modern iOS device (it's been tested as far back as the original
iPhone 4). Open up the SimpleExample/SimpleExample.xcodeproj, build and run.

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

Happily the framework includes the ability to retrain the network for custom objects that you care about.
If you have logos you need to pick out, machine parts you need to spot, or just want to be able to distinguish between different kinds of scenes like offices, beaches, mountains or forests, you should look at the [LearningExample](#learningexample) sample code.
It builds a custom layer on top of the basic neural network that responds to images you've trained it on, and allows you to embed the functionality in your own application easily.

There's also [this full how-to guide](https://github.com/jetpacapp/DeepBeliefSDK/wiki/How-to-recognize-custom-objects) on training and embedding your own custom object recognition code.

### Adding to an existing iOS application

To use the library in your own application: 

 - Add the DeepBelief.framework bundle to the Link Binary with Libraries build phase in your XCode project settings.
 - Add the system Accelerate.framework to your frameworks.
 - Add `#import <DeepBelief/DeepBelief.h>` to the top of the file you want to use the code in.

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

If you see errors related to `operator new` or similar messages at the linking stage, XCode may be skipping the standard C++ library, and that's needed by the DeepBelief.framework code. One workaround I've found is to include an empty .mm or .cpp file in the project to trick XCode into treating it as a C++ project.

## Getting Started on Android

I've been using Google's ADT toolchain. To get started import the AndroidExample into
their custom version of Eclipse, build and run it. Hopefully you should see a similar
result to the iPhone app, with live video and tags displayed. You'll need to hold the
phone in landscape orientation, look for the tag text and use that as your guide.

The Android implementation uses NEON SIMD instructions, so it may not work on
older phones, and will definitely not work on non-ARM devices. As a benchmark for 
expected performance, classification takes around 650ms on a Samsung Galaxy S5.

### Adding to an existing Android application

Under the hood the Android implementation uses a native C++ library that's linked to
Java applications using JNA. That means the process of including the code is a bit more
complex than on iOS. If you look at the [AndroidExample](#androidexample) sample code,
you'll see a 'libs' folder. This contains a deepbelief.jar file that has the Java interface
to the underlying native code, and then inside the armeabi there's jnidispatch.so which is
part of JNA and handles the mechanics of calling native functions, and libjpcnn.so which
implements the actual object recognition algorithm. You'll need to replicate this folder
structure and copy the files to your own application's source tree.

Once you've done that, you should be able to import the Java interface to the library:

`import com.jetpac.deepbelief.DeepBelief.JPCNNLibrary;`

This class contains a list of Java functions that correspond to exactly to the
[C interface functions](#api-reference). The class code is available in the AndroidLibrary
folder, and you should be able to rebuild it yourself by running ant, but here are the 
definitions using JNA types:

```java
Pointer jpcnn_create_network(String filename);
void jpcnn_destroy_network(Pointer networkHandle);
Pointer jpcnn_create_image_buffer_from_file(String filename);
void jpcnn_destroy_image_buffer(Pointer imageHandle);
Pointer jpcnn_create_image_buffer_from_uint8_data(byte[] pixelData, int width, int height, int channels, int rowBytes, int reverseOrder, int doRotate);
void jpcnn_classify_image(Pointer networkHandle, Pointer inputHandle, int doMultiSample, int layerOffset, PointerByReference outPredictionsValues, IntByReference outPredictionsLength, PointerByReference outPredictionsNames, IntByReference outPredictionsNamesLength);
void jpcnn_print_network(Pointer networkHandle);

Pointer jpcnn_create_trainer();
void jpcnn_destroy_trainer(Pointer trainerHandle);
void jpcnn_train(Pointer trainerHandle, float expectedLabel, float[] predictions, int predictionsLength);
Pointer jpcnn_create_predictor_from_trainer(Pointer trainerHandle);
void jpcnn_destroy_predictor(Pointer predictorHandle);
int jpcnn_save_predictor(String filename, Pointer predictorHandle);
Pointer jpcnn_load_predictor(String filename);
float jpcnn_predict(Pointer predictorHandle, Pointer predictions, int predictionsLength);
```

There are a few quirks to using the interface that the example code demonstrates how to work around. 
`jpcnn_create_network()` requires a standard filename path, but to distribute the network with an
application it needs to be an asset, and because that may be compressed and part of an archive, there's
no way to get a path to it. To fix that, `initDeepBelief()` copys the file to the application's data directory:

```java
AssetManager am = ctx.getAssets();
String baseFileName = "jetpac.ntwk";
String dataDir = ctx.getFilesDir().getAbsolutePath();
String networkFile = dataDir + "/" + baseFileName;
copyAsset(am, baseFileName, networkFile);
networkHandle = JPCNNLibrary.INSTANCE.jpcnn_create_network(networkFile);
```

This has some overhead obviously, so one optimization might be to check for the existence of the file and only
copy it over if it doesn't already exist.

`jpcnn_create_image_buffer_from_uint8_data()` needs a plain byte array, and the `classifyBitmap()` function
shows how you can extract what you need from a normal Bitmap object:

```java
final int width = bitmap.getWidth();
final int height = bitmap.getHeight();
final int pixelCount = (width * height);
final int bytesPerPixel = 4;
final int byteCount = (pixelCount * bytesPerPixel);
ByteBuffer buffer = ByteBuffer.allocate(byteCount);
bitmap.copyPixelsToBuffer(buffer);
byte[] pixels = buffer.array();
Pointer imageHandle = JPCNNLibrary.INSTANCE.jpcnn_create_image_buffer_from_uint8_data(pixels, width, height, 4, (4 * width), 0, 0);
```

Native objects are not garbage-collected, so you'll have to remember to explicitly call `jpcnn_destroy_image_buffer()`
and other calls on objects you've created through the library if you want to avoid memory leaks.

The rest of `classifyBitmap()` also demonstrates how to pull out the results as Java-accessible arrays from the JNA types.

## Getting Started on Linux

I've been using Ubuntu 12.04 and 14.04 on x86-64 platforms, but the library ships as
a simple .so with minimal dependencies, so hopefully it should work on most distros.

As long as you have git and the build-essentials packages installed, you should be able
to build an example by running the following commands in a terminal:

```shell
git clone https://github.com/jetpacapp/DeepBeliefSDK.git
cd DeepBeliefSDK/LinuxLibrary
sudo ./install.sh
cd ../examples/SimpleLinux/
make
./deepbelief 
```

If [the example program](#simplelinux) ran successfully, the output should look like this:

```shell
0.016994	wool
0.016418	cardigan
0.010924	kimono
0.010713	miniskirt
0.014307	crayfish
0.015663	brassiere
0.014216	harp
0.017052	sandal
0.024082	holster
0.013580	velvet
0.057286	bonnet
0.018848	stole
0.028298	maillot
0.010915	gown
0.073035	wig
0.012413	hand blower
0.031052	stage
0.027875	umbrella
0.012592	sarong
```

It's analyzing the default Lena image, and giving low probabilities of a wig
and a bonnet, which isn't too crazy. You can pass in a command-line argument
to analyze your own images, and the results are tab separated text, so you can
pipe the results into other programs for further processing.

### Adding to an existing Linux application

To use the library in your own application, first make sure you've run the install.sh script
in AndroidLibrary/ to install the libjpcnn.so in /usr/lib, and libjpcnn.h in /usr/include, as
described in [Getting Started on Linux](#getting-started-on-linux).

Then you should be able to access [all the API functions](#api-reference) by including the libjpcnn.h header, eg:

```c
#include <libjpcnn.h>
```

Here's how you would run a basic classification of a single image, from the [SimpleLinux example](#simplelinux):

```c
  networkHandle = jpcnn_create_network(NETWORK_FILE_NAME);
  imageHandle = jpcnn_create_image_buffer_from_file(imageFileName);

  jpcnn_classify_image(networkHandle, imageHandle, 0, 0, &predictions, &predictionsLength, &predictionsLabels, &predictionsLabelsLength);

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
```

## Getting Started on OS X

Load the examples/OSXExample/MyRecorder.xcodeproj XCode project, build, and run.
On any machine with a webcam, you should see a window appear showing live video.
Move the webcam until it has a clear view of an object like a wine bottle, glass, mug, or a computer keyboard, and you should start to see overlaid labels and percentages.

### Adding to an existing OS X application

The DeepBelief.framework you'll need is in the OSXLibrary folder.
Since installing frameworks in a shared location can be a pain, and Apple recommends keeping applications as self-contained as possible, it's designed to be bundled inside your app folder.
The [OS X Example](#osxexample) sample code uses this approach, and is a good starting point for understanding the process.
It has a symbolic link back to the framework, but you'll probably want to copy the library into your own source tree.
[Apple's documentation on bundling private frameworks](https://developer.apple.com/library/mac/documentation/macosx/conceptual/BPFrameworks/Tasks/CreatingFrameworks.html#//apple_ref/doc/uid/20002258-106880) is the best documentation for the whole process, but here's the summary of what you'll need to do:

 * Copy DeepBelief.framework into your source tree
 * Drag it into the Frameworks folder of your project in the XCode navigator.
 * Add it to the "Link Binary with Libraries" build phase in the project settings.
 * Add a new "Copy Files Build Phase" to the project build phases.
 * Add the framework as a new file in that build phase, with the destination as "Frameworks".

Once you've done that, you should be able to build your app, and then "Show package contents" on the built product should show DeepBelief.framework inside the Contents/Frameworks folder.

At that point, just add `#import <DeepBelief/DeepBelief.h>` and all of the code you need should be identical to the snippets shown in [the iOS guide](#adding-to-an-existing-ios-application).

### Using with OpenCV

It's pretty straightforward to use DeepBelief together with OpenCV, you just need to convert the images over.
There's [sample code showing the whole process](#simpleopencv), but the heart of it is this image format conversion:

```c++
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
```

Once you've done that, you can run the image classification and prediction as normal on the image handle.
[The sample code](#simpleopencv) has some other convenience classes too, to help make using the library in C++ a bit easier.
If you're using the Java interface, the same sort of call sequence works to handle the conversion, though you'll need `byte[]` arrays and you'll have to call `image.get(0, 0, pixels)` to actually get the raw image data you need.

## Getting Started on a Raspberry Pi

The library is available as a Raspbian .so library in the RaspberryPiLibrary folder.
Using it is very similar to ordinary Linux, and you can follow most of the [same instructions](#getting-started-on-linux), substituting the install.sh in the Pi folder.
The biggest difference is that the Pi library uses the GPU to handle a lot of the calculations, so you need to run [the example program](#simplelinux) as a super user, e.g. `sudo ./deepbelief`.
This optimization allows an image to be recognized on a stock Pi in around five seconds, and in three seconds with a boosted GPU clock rate.

## Examples

All of the sample code projects are included in the 'examples' folder in this git repository.

 - [SimpleiOS](#simpleios)
 - [LearningExample](#learningexample)
 - [SavedModelExample](#savedmodelexample)
 - [AndroidExample](#androidexample)
 - [SimpleLinux](#simplelinux)
 - [OSXExample](#osxexample)
 - [SimpleOpenCV](#simpleopencv)

### SimpleiOS

This is a self-contained iOS application that shows you how to load the neural network parameters, and process live video to estimate the probability that one of the 1,000 pre-defined Imagenet objects are present.
The code is largely based on the [SquareCam Apple sample application](https://developer.apple.com/library/ios/samplecode/squarecam/Introduction/Intro.html), which is fairly old and contains some ugly code.
If you look for `jpcnn_*` calls in SquareCamViewController.m you should be able to follow the sequence of first loading the network, applying it to video frames as they arrive, and destroying the objects once you're all done.

### LearningExample

This application allows you to apply the image recognition code to custom objects you care about. It demonstrates how to capture positive and negative examples, feed them into a trainer to create a prediction model, and then apply that prediction model to the live camera feed.
It can be a bit messy thanks to all the live video feed code, but if you look for `jpcnn_*` you'll be able to spot the main flow. Once a prediction model has been fully trained, the parameters are written to the XCode console so they can be used as pre-trained predictors.

### SavedModelExample

This shows how you can use a custom prediction model that you've built using the [LearningExample](#learningexample) sample code. 
I've included the simple 'wine_bottle_predictor.txt' that I quickly trained on a bottle of wine, you should be able to run it yourself and see the results of that model's prediction on your own images.

### AndroidExample

A basic Android application that applies the classification algorithm to live video from the phone's camera. The first thing it does after initialization is analyze the standard image-processing image of Lena, you should see log output from that first.
After that it continuously analyzes incoming camera frames, both displaying the found labels on screen and printing them to the console.

### SimpleLinux

This is a small command line tool that shows how you can load a network file and classify an image using the default Imagenet categories.
If you run it with no arguments, it looks for lena.png and analyzes that, otherwise it tries to load the file name in the first argument as its input image.

The network file name is hardcoded to "jetpac.ntwk" in the current folder.
In a real application you'll want to set that yourself, either hard-coding it to a known absolute location for the file, or passing it in dynamically as an argument or environment variable.

The output of the tool is tab-separated lines, with the probability first followed by the imagenet label, so you can sort and process it easily through pipes on the command line.

### OSXExample

This project is based on [Apple's MyRecorder sample code](https://developer.apple.com/library/mac/samplecode/MYRecorder/Introduction/Intro.html), which is both quite old and fairly gnarly thanks to its use of QTKit!
The complexity is mostly in the way it accesses the webcam, and converts the supplied image down to a simple array of RGB bytes to feed into the neural network code. 
If you search for 'jpcnn' in the code, you'll see the calls to the library nestled amongst all the plumbing for the interface and the video, they should be fairly straightforward.

The main steps are loading the 'jetpac.ntwk' neural network, that's included as a resource in the app, then extracting an image from the video, classifying it, and displaying the found labels in the UI.
When you build and run the project, you should see a window appear with the webcam view in it, and any found labels overlaid on top. You'll also see some performance stats being output to the console - on my mid-2012 Macbook Pro it takes around 60ms to do the calculations.

### SimpleOpenCV

This is a basic Linux command-line tool that shows how OpenCV and the DeepBelief framework can work together.
The main() function uses C++ classes defined in deepbeliefopencv.h to load a network, then it creates an OpenCV image from either lena.png or another file supplied on the command line.
A wrapper class for the library's image handle object is then used to convert the OpenCV image into one the DeepBelief framework can analyze.
The classification is run on that image, and the found labels are printed out.

If you're doing a lot of work with OpenCV, the most crucial part for you is probably the conversion of the image objects between the two systems. 
That's defined in deepbeliefopencv.cpp in the `Image::Image(const cv::Mat& image)` constructor, and [the section on using OpenCV](#using-with-opencv) covers what's going on in the actual code.

## Networks

There are currently three pre-built models available in the networks folder.
jetpac.ntwk is the in-house model used here at Jetpac, and it's licensed under the same BSD conditions as the rest of the project. It has a few oddities, like only 999 labels (a file truncation problem I discovered too late during training) but has served us well and is a good place to start.

The excellent [libccv](http://libccv.org) project also made a couple of networks available under a [Creative Commons Attribution 4.0 International License](http://creativecommons.org/licenses/by/4.0/). I've converted them over into a binary format, and they're in the networks folder as ccv2010.ntwk and ccv2012.ntwk. You should be able to substitute these in anywhere you'd use jetpac.ntwk. The 2012 file has very similar labels to our original, and the 2010 is an older architecture. You may notice slightly slower performance, the arrangement of the layers is a bit different (in technical terms the local-response normalization happens before the max-pooling in these models, which is more expensive since there's more data to normalize), but the accuracy of the 2012 model especially is good. One common technique in the academic world is to take multiple models and merge their votes for higher accuracy, so one application of the multiple models might be improved accuracy.

## API Reference

Because we reuse the same code across a lot of different platforms, we use a
plain-old C interface to our library. All of the handles to different objects are
opaque pointers, and you have to explictly call the `*_destroy_*` function on any
handles that have been returned from `*_create_*` calls if you want to avoid memory
leaks. Input images are created from raw arrays of 8-bit RGB data, you can see how 
to build those from iOS types by searching for `jpcnn_create_image_buffer()` in the 
sample code.

The API is broken up into two sections. The first gives you access to one of the pre-trained neural networks you'll find in the networks folder.
These have been trained on 1,000 Imagenet categories, and the output will give you a decent general idea of what's in an image.

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

## FAQ

### Is this available for platforms other than iOS, Android, OS X, and Linux x86-64?

Not right now. I hope to make it available on other devices like the Raspberry Pi in the future. I recommend checking out [Caffe](https://github.com/BVLC/caffe), [OverFeat](http://cilvr.nyu.edu/doku.php?id=software:overfeat:start) and [libCCV](http://libccv.org) if you're on the desktop too, they're great packages.

### Is the source available?

Not at the moment. The compiled library and the neural network parameter set are freely reusable in your own apps under the BSD license though.

### Can I train my own networks?

There aren't any standard formats for sharing large neural networks unfortunately, so there's no easy way to import other CNNs into the app. The [custom training](https://github.com/jetpacapp/DeepBeliefSDK/wiki/How-to-recognize-custom-objects) should help you apply the included pre-trained network to your own problems to a large extent though.

## More Information

Join the [Deep Belief Developers email list](https://groups.google.com/group/deep-belief-developers) to find out more about the practical details of implementing deep learning.

## License

The binary framework and jetpac.ntwk network parameter file are under the BSD three-clause
license, included in this folder as LICENSE.

The ccv2010.ntwk and ccv2012.ntwk network models were converted from files created as part of the [LibCCV project](http://libccv.org/) and are licensed under the Creative Commons Attribution 4.0 International License. To view a copy of this license, visit [http://creativecommons.org/licenses/by/4.0/](http://creativecommons.org/licenses/by/4.0/).

## Credits

Big thanks go to:

 - [Daniel Nouri](http://danielnouri.org/) for his invaluable help on CNNs.
 - [Alex Krizhevsky, Ilya Sutskever, and Geoffrey Hinton](http://www.cs.toronto.edu/~hinton/absps/imagenet.pdf) for their ground-breaking work on ConvNet.
 - [Yann LeCun](https://plus.google.com/+YannLeCunPhD/posts) for his pioneering research and continuing support of the field.
 - The Berkeley Vision and Learning Center for their work on [lightweight custom training of CNNs](https://github.com/BVLC/caffe).
 - My colleagues [Cathrine](https://twitter.com/lindblomsten), [Dave](https://twitter.com/davefearon), [Julian](https://twitter.com/juliangreensf), and [Sophia](https://twitter.com/spara) at [Jetpac](https://www.jetpac.com/) for all their help.
 
[Pete Warden](https://twitter.com/petewarden)
