package com.example.cam;

/**
 * @author Jose Davis Nidhin
 */

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.ByteBuffer;

import android.app.Activity;
import android.content.Context;
import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.hardware.Camera;
import android.hardware.Camera.AutoFocusCallback;
import android.hardware.Camera.PictureCallback;
import android.hardware.Camera.ShutterCallback;
import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceView;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnLongClickListener;
import android.view.ViewGroup.LayoutParams;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.FrameLayout;

import com.jetpac.deepbelief.DeepBelief;
import com.jetpac.deepbelief.DeepBelief.JPCNNLibrary;
import com.sun.jna.Pointer;
import com.sun.jna.ptr.IntByReference;
import com.sun.jna.ptr.PointerByReference;

public class CamTestActivity extends Activity {
	private static final String TAG = "CamTestActivity";
	Preview preview;
	Button buttonClick;
	Camera camera;
	String fileName;
	Activity act;
	Context ctx;
	Pointer networkHandle = null;

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		ctx = this;
		act = this;
		requestWindowFeature(Window.FEATURE_NO_TITLE);
		getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);

		setContentView(R.layout.main);
		
		preview = new Preview(this, (SurfaceView)findViewById(R.id.surfaceView));
		preview.setLayoutParams(new LayoutParams(LayoutParams.FILL_PARENT, LayoutParams.FILL_PARENT));
		((FrameLayout) findViewById(R.id.preview)).addView(preview);
		preview.setKeepScreenOn(true);
		
		buttonClick = (Button) findViewById(R.id.buttonClick);
		
		buttonClick.setOnClickListener(new OnClickListener() {
			public void onClick(View v) {
				//				preview.camera.takePicture(shutterCallback, rawCallback, jpegCallback);
				camera.takePicture(shutterCallback, rawCallback, jpegCallback);
			}
		});
		
		buttonClick.setOnLongClickListener(new OnLongClickListener(){
			@Override
			public boolean onLongClick(View arg0) {
				camera.autoFocus(new AutoFocusCallback(){
					@Override
					public void onAutoFocus(boolean arg0, Camera arg1) {
						//camera.takePicture(shutterCallback, rawCallback, jpegCallback);
					}
				});
				return true;
			}
		});
		initDeepBelief();
	}

	@Override
	protected void onResume() {
		super.onResume();
		//      preview.camera = Camera.open();
		camera = Camera.open();
		camera.startPreview();
		preview.setCamera(camera);
	}

	@Override
	protected void onPause() {
		if(camera != null) {
			camera.stopPreview();
			preview.setCamera(null);
			camera.release();
			camera = null;
		}
		super.onPause();
	}

	private void resetCam() {
		camera.startPreview();
		preview.setCamera(camera);
	}

	ShutterCallback shutterCallback = new ShutterCallback() {
		public void onShutter() {
			// Log.d(TAG, "onShutter'd");
		}
	};

	PictureCallback rawCallback = new PictureCallback() {
		public void onPictureTaken(byte[] data, Camera camera) {
			// Log.d(TAG, "onPictureTaken - raw");
		}
	};

	PictureCallback jpegCallback = new PictureCallback() {
		public void onPictureTaken(byte[] data, Camera camera) {
//			FileOutputStream outStream = null;
//			try {
//				// Write to SD Card
//				fileName = String.format("/sdcard/camtest/%d.jpg", System.currentTimeMillis());
//				outStream = new FileOutputStream(fileName);
//				outStream.write(data);
//				outStream.close();
//				Log.d(TAG, "onPictureTaken - wrote bytes: " + data.length);

				resetCam();

//			} catch (FileNotFoundException e) {
//				e.printStackTrace();
//			} catch (IOException e) {
//				e.printStackTrace();
//			} finally {
//			}
			Log.d(TAG, "onPictureTaken - jpeg");
		}
	};

	void initDeepBelief() {
		AssetManager am = ctx.getAssets();
		String baseFileName = "jetpac.ntwk";
		String dataDir = ctx.getFilesDir().getAbsolutePath();
		String networkFile = dataDir + "/" + baseFileName;
		copyAsset(am, baseFileName, networkFile);
	    networkHandle = JPCNNLibrary.INSTANCE.jpcnn_create_network(networkFile);

	    Bitmap lenaBitmap = getBitmapFromAsset("lena.png");
	    classifyBitmap(lenaBitmap);
	}
	
	void classifyBitmap(Bitmap bitmap) {
	    final int width = bitmap.getWidth();
	    final int height = bitmap.getHeight();
	    final int pixelCount = (width * height);
	    final int bytesPerPixel = 4;
	    final int byteCount = (pixelCount * bytesPerPixel);
	    ByteBuffer buffer = ByteBuffer.allocate(byteCount);
	    bitmap.copyPixelsToBuffer(buffer);
	    byte[] pixels = buffer.array();
	    Pointer imageHandle = JPCNNLibrary.INSTANCE.jpcnn_create_image_buffer_from_uint8_data(pixels, width, height, 4, (4 * width), 0, 0);

	    PointerByReference predictionsValuesRef = new PointerByReference();
	    IntByReference predictionsLengthRef = new IntByReference();
	    PointerByReference predictionsNamesRef = new PointerByReference();
	    IntByReference predictionsNamesLengthRef = new IntByReference();
	    JPCNNLibrary.INSTANCE.jpcnn_classify_image(
	      networkHandle,
	      imageHandle,
	      0,
	      0,
	      predictionsValuesRef,
	      predictionsLengthRef,
	      predictionsNamesRef,
	      predictionsNamesLengthRef);

	    Pointer predictionsValuesPointer = predictionsValuesRef.getValue();
	    final int predictionsLength = predictionsLengthRef.getValue();
	    Pointer predictionsNamesPointer = predictionsNamesRef.getValue();
	    final int predictionsNamesLength = predictionsNamesLengthRef.getValue();

        System.err.println(String.format("predictionsLength = %d", predictionsLength));
	    
	    float[] predictionsValues = predictionsValuesPointer.getFloatArray(0, predictionsLength);
	    Pointer[] predictionsNames = predictionsNamesPointer.getPointerArray(0); 
	    for (int index = 0; index < predictionsLength; index += 1) {
	    	final float predictionValue = predictionsValues[index];
	    	if (predictionValue > 0.01f) {
	    		byte[] nameBytes = predictionsNames[index].getByteArray(0, 10);
	    		String name = new String(nameBytes);
	            System.err.println(String.format("%s = %f", name, predictionValue));	    		
	    	}
	    }
	    
	}
	
    private static boolean copyAsset(AssetManager assetManager,
            String fromAssetPath, String toPath) {
        InputStream in = null;
        OutputStream out = null;
        try {
          in = assetManager.open(fromAssetPath);
          new File(toPath).createNewFile();
          out = new FileOutputStream(toPath);
          copyFile(in, out);
          in.close();
          in = null;
          out.flush();
          out.close();
          out = null;
          return true;
        } catch(Exception e) {
            e.printStackTrace();
            return false;
        }
    }

    private static void copyFile(InputStream in, OutputStream out) throws IOException {
        byte[] buffer = new byte[1024];
        int read;
        while((read = in.read(buffer)) != -1){
          out.write(buffer, 0, read);
        }
    }

    private Bitmap getBitmapFromAsset(String strName) {
        AssetManager assetManager = getAssets();
        InputStream istr = null;
        try {
            istr = assetManager.open(strName);
        } catch (IOException e) {
            e.printStackTrace();
        }
        Bitmap bitmap = BitmapFactory.decodeStream(istr);
        return bitmap;
    }
}
