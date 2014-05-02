//
//  MyRecorderController.m
//  MyRecorder

#import "MyRecorderController.h"

#include <sys/time.h>

#import <DeepBelief/DeepBelief.h>

@implementation MyRecorderController

- (void)awakeFromNib
{
  NSString* networkPath = [[NSBundle mainBundle] pathForResource:@"jetpac" ofType:@"ntwk"];
  if (networkPath == NULL) {
    fprintf(stderr, "Couldn't find the neural network parameters file - did you add it as a resource to your application?\n");
    assert(false);
  }
  network = jpcnn_create_network([networkPath UTF8String]);
  assert(network != NULL);

  CALayer* rootLayer = [[CALayer alloc] init];
  [mainView setLayer: rootLayer];
  [mainView setWantsLayer: YES];

  labelLayers = [[NSMutableArray alloc] init];
  oldPredictionValues = [[NSMutableDictionary alloc] init];

	mCaptureSession = [[QTCaptureSession alloc] init];
    
	BOOL success = NO;
	NSError *error;

  QTCaptureDevice *videoDevice = [QTCaptureDevice defaultInputDeviceWithMediaType:QTMediaTypeVideo];
  success = [videoDevice open:&error];
	if (!success) {
		videoDevice = [QTCaptureDevice defaultInputDeviceWithMediaType:QTMediaTypeMuxed];
		success = [videoDevice open:&error];
  }
    
  if (!success) {
    videoDevice = nil;
    assert(false);
    return;
  }

  mCaptureVideoDeviceInput = [[QTCaptureDeviceInput alloc] initWithDevice:videoDevice];
  success = [mCaptureSession addInput:mCaptureVideoDeviceInput error:&error];
  if (!success) {
    assert(false);
  }

  output = [[QTCaptureDecompressedVideoOutput alloc] init];
  [output setDelegate:self];
  success = [mCaptureSession addOutput:output error:&error];

  [output setPixelBufferAttributes:[NSDictionary
    dictionaryWithObjectsAndKeys:
    [NSNumber
    numberWithDouble:640.0], (id)kCVPixelBufferWidthKey,
    [NSNumber
    numberWithDouble:480.0], (id)kCVPixelBufferHeightKey,
    [NSNumber
    numberWithUnsignedInt:kCVPixelFormatType_32ARGB],
    (id)kCVPixelBufferPixelFormatTypeKey,
  nil]];

  [mCaptureView setCaptureSession:mCaptureSession];
  
  [mCaptureSession startRunning];

}

- (void)windowWillClose:(NSNotification *)notification {
	[mCaptureSession stopRunning];
    
  if ([[mCaptureVideoDeviceInput device] isOpen])
      [[mCaptureVideoDeviceInput device] close];
  
  if ([[mCaptureAudioDeviceInput device] isOpen])
      [[mCaptureAudioDeviceInput device] close];
}

- (void)dealloc {
	[mCaptureSession release];
	[mCaptureVideoDeviceInput release];
  [mCaptureAudioDeviceInput release];
	[mCaptureMovieFileOutput release];

  jpcnn_destroy_network(network);

	[super dealloc];
}

- (void)captureOutput:(QTCaptureOutput *)captureOutput didOutputVideoFrame:(CVPixelBufferRef)videoFrame withSampleBuffer:(QTSampleBuffer *)sampleBuffer fromConnection:(QTCaptureConnection *)connection
{
  [self runCNNOnFrame: videoFrame];
}

- (void)runCNNOnFrame: (CVPixelBufferRef) pixelBuffer
{
  assert(pixelBuffer != NULL);

	OSType sourcePixelFormat = CVPixelBufferGetPixelFormatType( pixelBuffer );
	assert( kCVPixelFormatType_32ARGB == sourcePixelFormat );
  const int sourceRowBytes = (int)CVPixelBufferGetBytesPerRow( pixelBuffer );
  const int width = (int)CVPixelBufferGetWidth( pixelBuffer );
  const int height = (int)CVPixelBufferGetHeight( pixelBuffer );
  CVPixelBufferLockBaseAddress( pixelBuffer, 0 );
  unsigned char* sourceStartAddr = CVPixelBufferGetBaseAddress( pixelBuffer );
  const int sourceChannels = 4;
  const int destChannels = 3;
  const int destRowBytes = (width * destChannels);
  const size_t destByteCount = (destRowBytes * height);
  uint8_t* destData = (uint8_t*)(malloc(destByteCount));

  for (int y = 0; y < height; y += 1) {
    uint8_t* source = (sourceStartAddr + (y * sourceRowBytes));
    uint8_t* sourceRowEnd = (source + (width * sourceChannels));
    uint8_t* dest = (destData + (y * destRowBytes));
    while (source < sourceRowEnd) {
      dest[0] = source[1];
      dest[1] = source[2];
      dest[2] = source[3];
      source += sourceChannels;
      dest += destChannels;
    }
  }

  void* cnnInput = jpcnn_create_image_buffer_from_uint8_data(destData, width, height, destChannels, destRowBytes, 0, 0);

  free(destData);;

  float* predictions;
  int predictionsLength;
  char** predictionsLabels;
  int predictionsLabelsLength;

  struct timeval start;
  gettimeofday(&start, NULL);
  jpcnn_classify_image(network, cnnInput, JPCNN_RANDOM_SAMPLE, 0, &predictions, &predictionsLength, &predictionsLabels, &predictionsLabelsLength);
  struct timeval end;
  gettimeofday(&end, NULL);
  const long seconds  = end.tv_sec  - start.tv_sec;
  const long useconds = end.tv_usec - start.tv_usec;
  const float duration = ((seconds) * 1000 + useconds/1000.0) + 0.5;
  NSLog(@"Took %f ms", duration);

  jpcnn_destroy_image_buffer(cnnInput);

  const float timeSinceUpdate = ((end.tv_sec - lastUpdateTime.tv_sec) * 1000 + (end.tv_usec - lastUpdateTime.tv_usec)/1000.0) + 0.5;
  // Throttle the update frequency
  if (timeSinceUpdate > 0.3f) {
    lastUpdateTime = end;

    NSMutableDictionary* newValues = [NSMutableDictionary dictionary];
    for (int index = 0; index < predictionsLength; index += 1) {
      const float predictionValue = predictions[index];
      if (predictionValue > 0.05f) {
        char* label = predictionsLabels[index % predictionsLabelsLength];
        NSString* labelObject = [NSString stringWithCString: label];
        NSNumber* valueObject = [NSNumber numberWithFloat: predictionValue];
        [newValues setObject: valueObject forKey: labelObject];
      }
    }
    dispatch_async(dispatch_get_main_queue(), ^(void) {
      [self setPredictionValues: newValues];
    });
  }
}

- (void)setPredictionValues: (NSDictionary*) newValues {
  const float decayValue = 0.75f;
  const float updateValue = 0.25f;
  const float minimumThreshold = 0.01f;

  NSMutableDictionary* decayedPredictionValues = [[NSMutableDictionary alloc] init];
  for (NSString* label in oldPredictionValues) {
    NSNumber* oldPredictionValueObject = [oldPredictionValues objectForKey:label];
    const float oldPredictionValue = [oldPredictionValueObject floatValue];
    const float decayedPredictionValue = (oldPredictionValue * decayValue);
    if (decayedPredictionValue > minimumThreshold) {
      NSNumber* decayedPredictionValueObject = [NSNumber numberWithFloat: decayedPredictionValue];
      [decayedPredictionValues setObject: decayedPredictionValueObject forKey:label];
    }
  }
  [oldPredictionValues release];
  oldPredictionValues = decayedPredictionValues;

  for (NSString* label in newValues) {
    NSNumber* newPredictionValueObject = [newValues objectForKey:label];
    NSNumber* oldPredictionValueObject = [oldPredictionValues objectForKey:label];
    if (!oldPredictionValueObject) {
      oldPredictionValueObject = [NSNumber numberWithFloat: 0.0f];
    }
    const float newPredictionValue = [newPredictionValueObject floatValue];
    const float oldPredictionValue = [oldPredictionValueObject floatValue];
    const float updatedPredictionValue = (oldPredictionValue + (newPredictionValue * updateValue));
    NSNumber* updatedPredictionValueObject = [NSNumber numberWithFloat: updatedPredictionValue];
    [oldPredictionValues setObject: updatedPredictionValueObject forKey:label];
  }
  NSArray* candidateLabels = [NSMutableArray array];
  for (NSString* label in oldPredictionValues) {
    NSNumber* oldPredictionValueObject = [oldPredictionValues objectForKey:label];
    const float oldPredictionValue = [oldPredictionValueObject floatValue];
    if (oldPredictionValue > 0.05f) {
      NSDictionary *entry = @{
        @"label" : label,
        @"value" : oldPredictionValueObject
      };
      candidateLabels = [candidateLabels arrayByAddingObject: entry];
    }
  }
  NSSortDescriptor *sort = [NSSortDescriptor sortDescriptorWithKey:@"value" ascending:NO];
  NSArray* sortedLabels = [candidateLabels sortedArrayUsingDescriptors:[NSArray arrayWithObject:sort]];

  const float leftMargin = 10.0f;
  const float topMargin = 10.0f;

  const float valueWidth = 48.0f;
  const float valueHeight = 26.0f;

  const float labelWidth = 246.0f;
  const float labelHeight = 26.0f;

  const float labelMarginX = 5.0f;
  const float labelMarginY = 5.0f;

  [self removeAllLabelLayers];

  int labelCount = 0;
  for (NSDictionary* entry in sortedLabels) {
    NSString* label = [entry objectForKey: @"label"];
    NSNumber* valueObject =[entry objectForKey: @"value"];
    const float value = [valueObject floatValue];

    const float originY = 400 - (topMargin + ((labelHeight + labelMarginY) * labelCount));

    const int valuePercentage = (int)roundf(value * 100.0f);

    const float valueOriginX = leftMargin;
    NSString* valueText = [NSString stringWithFormat:@"%d%%", valuePercentage];

    [self addLabelLayerWithText:valueText
      originX:valueOriginX originY:originY
      width:valueWidth height:valueHeight
      alignment: kCAAlignmentRight];

    const float labelOriginX = (leftMargin + valueWidth + labelMarginX);

    [self addLabelLayerWithText: [label capitalizedString]
      originX: labelOriginX originY: originY
      width: labelWidth height: labelHeight
      alignment:kCAAlignmentLeft];

    labelCount += 1;
    if (labelCount > 4) {
      break;
    }
  }
}

- (void) removeAllLabelLayers {
  for (CATextLayer* layer in labelLayers) {
    [layer removeFromSuperlayer];
  }
  [labelLayers removeAllObjects];
}

- (void) addLabelLayerWithText: (NSString*) text
  originX:(float) originX originY:(float) originY
  width:(float) width height:(float) height
  alignment:(NSString*) alignment
 {
  NSString* const font = @"Menlo-Regular";
  const float fontSize = 20.0f;

  const float marginSizeX = 5.0f;
  const float marginSizeY = 2.0f;

  const CGRect backgroundBounds = CGRectMake(originX, originY, width, height);

  const CGRect textBounds = CGRectMake((originX + marginSizeX), (originY + marginSizeY),
    (width - (marginSizeX * 2)), (height - (marginSizeY * 2)));

  CATextLayer* background = [CATextLayer layer];
  [background setBackgroundColor: [NSColor blackColor].CGColor];
  [background setOpacity:0.5f];
  [background setFrame: backgroundBounds];
  background.cornerRadius = 5.0f;

  [[mainView layer] addSublayer: background];
  [labelLayers addObject: background];

  CATextLayer *layer = [CATextLayer layer];
  [layer setForegroundColor: [NSColor whiteColor].CGColor];
  [layer setFrame: textBounds];
  [layer setAlignmentMode: alignment];
  [layer setWrapped: YES];
  [layer setFont: font];
  [layer setFontSize: fontSize];
  [layer setString: text];

  [[mainView layer] addSublayer: layer];
  [labelLayers addObject: layer];
}

@end
