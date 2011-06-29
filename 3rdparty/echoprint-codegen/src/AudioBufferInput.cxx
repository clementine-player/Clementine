//
//  echoprint-codegen
//  Copyright 2011 The Echo Nest Corporation. All rights reserved.
//

#include <string.h>
#include <limits.h>
#include <assert.h>


#include "AudioBufferInput.h"

AudioBufferInput::AudioBufferInput() { }

void AudioBufferInput::SetBuffer(const float* pBuffer, uint numSamples) {
    _NumberSamples = numSamples;
    _pSamples = new float[_NumberSamples]; // base-class destructor will clean this up.
    memcpy(_pSamples, pBuffer, numSamples*sizeof(float));
}

void AudioBufferInput::SaveBuffer(const char*filename) {
    FILE *out = fopen(filename,"wb");
    fwrite(&_NumberSamples, sizeof(int), 1, out);
    uint mn = 1;
    fwrite(&mn, sizeof(int), 1, out);
    fwrite(_pSamples, 4, _NumberSamples, out);
    fclose(out);
}

