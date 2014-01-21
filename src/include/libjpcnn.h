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

void* jpcnn_create_network(const char* filename);
void jpcnn_destroy_network(void* networkHandle);
void* jpcnn_create_image_buffer_from_file(const char* filename);
void jpcnn_destroy_image_buffer(void* imageHandle);
void* jpcnn_create_image_buffer_from_uint8_data(unsigned char* pixelData, int width, int height, int channels, int rowBytes, int reverseOrder, int doRotate);
void jpcnn_classify_image(void* networkHandle, void* inputHandle, int doMultiSample, int layerOffset, float** outPredictionsValues, int* outPredictionsLength, char*** outPredictionsNames, int* outPredictionsNamesLength);

#ifdef __cplusplus
}
#endif // __cplusplus
