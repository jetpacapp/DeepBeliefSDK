//
//  libjpcnn.h
//  jpcnn
//
//  Implements the external library interface to the Jetpac CNN code.
//
//  Created by Peter Warden on 1/15/14.
//  Copyright (c) 2014 Jetpac, Inc. All rights reserved.
//

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define JPCNN_MULTISAMPLE      (1 << 0)
#define JPCNN_RANDOM_SAMPLE    (1 << 1)

void* jpcnn_create_network(const char* filename);
void jpcnn_destroy_network(void* networkHandle);
void* jpcnn_create_image_buffer_from_file(const char* filename);
void jpcnn_destroy_image_buffer(void* imageHandle);
void* jpcnn_create_image_buffer_from_uint8_data(unsigned char* pixelData, int width, int height, int channels, int rowBytes, int reverseOrder, int doRotate);
void jpcnn_classify_image(void* networkHandle, void* inputHandle, unsigned int flags, int layerOffset, float** outPredictionsValues, int* outPredictionsLength, char*** outPredictionsNames, int* outPredictionsNamesLength);
void jpcnn_print_network(void* networkHandle);

void* jpcnn_create_trainer();
void jpcnn_destroy_trainer(void* trainerHandle);
void jpcnn_train(void* trainerHandle, float expectedLabel, float* predictions, int predictionsLength);
void* jpcnn_create_predictor_from_trainer(void* trainerHandle);
void jpcnn_destroy_predictor(void* predictorHandle);
int jpcnn_save_predictor(const char* filename, void* predictorHandle);
void* jpcnn_load_predictor(const char* filename);
void jpcnn_print_predictor(void* predictorHandle);
float jpcnn_predict(void* predictorHandle, float* predictions, int predictionsLength);

#ifdef __cplusplus
}
#endif // __cplusplus
