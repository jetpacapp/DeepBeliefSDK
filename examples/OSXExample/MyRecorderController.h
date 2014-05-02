//
//  MyRecorderController.h
//  MyRecorder

#import <Cocoa/Cocoa.h>
#import <QTKit/QTkit.h>

@interface MyRecorderController : NSObject {
  IBOutlet QTCaptureView *mCaptureView;
  IBOutlet NSView* mainView;
  QTCaptureSession            *mCaptureSession;
  QTCaptureMovieFileOutput    *mCaptureMovieFileOutput;
  QTCaptureDeviceInput        *mCaptureVideoDeviceInput;
  QTCaptureDeviceInput        *mCaptureAudioDeviceInput;
  QTCaptureDecompressedVideoOutput *output;
  void* network;
  NSMutableDictionary* oldPredictionValues;
  NSMutableArray* labelLayers;
  struct timeval lastUpdateTime;
}

@end
