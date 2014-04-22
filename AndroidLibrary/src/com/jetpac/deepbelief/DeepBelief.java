package com.jetpac.deepbelief;

import com.sun.jna.*;
import com.sun.jna.ptr.*;

public class DeepBelief {

  static final String NATIVE_LIBRARY_NAME = "jpcnn";

  public interface JPCNNLibrary extends Library {
    JPCNNLibrary INSTANCE = (JPCNNLibrary)
      Native.loadLibrary(DeepBelief.NATIVE_LIBRARY_NAME, JPCNNLibrary.class);

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
  }

}
