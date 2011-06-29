//
//  echoprint-codegen
//  Copyright 2011 The Echo Nest Corporation. All rights reserved.
//


#include "Whitening.h"
#include "AudioStreamInput.h"

Whitening::Whitening(AudioStreamInput* pAudio) {
    _pSamples = pAudio->getSamples();
    _NumSamples = pAudio->getNumSamples();
    Init();
}

Whitening::Whitening(const float* pSamples, uint numSamples) :
    _pSamples(pSamples), _NumSamples(numSamples) {
    Init();
}

Whitening::~Whitening() {
    free(_R);
    free(_Xo);
    free(_ai);
    free(_whitened);
}

void Whitening::Init() {
    int i;
    _p = 40;

    _R = (float *)malloc((_p+1)*sizeof(float));
    for (i = 0; i <= _p; ++i)  { _R[i] = 0.0; }
    _R[0] = 0.001;

    _Xo = (float *)malloc((_p+1)*sizeof(float));
    for (i = 0; i < _p; ++i)  { _Xo[i] = 0.0; }

    _ai = (float *)malloc((_p+1)*sizeof(float));
    _whitened = (float*) malloc(sizeof(float)*_NumSamples);
}

void Whitening::Compute() {
    int blocklen = 10000;
    int i, newblocklen;
    for(i=0;i<(int)_NumSamples;i=i+blocklen) {
        if (i+blocklen >= (int)_NumSamples) {
            newblocklen = _NumSamples -i - 1;
        } else { newblocklen = blocklen; }
        ComputeBlock(i, newblocklen);
    }
}

void Whitening::ComputeBlock(int start, int blockSize) {
    int i, j;
    float alpha, E, ki;
    float T = 8;
    alpha = 1.0/T;

    // calculate autocorrelation of current block

    for (i = 0; i <= _p; ++i) {
        float acc = 0;
        for (j = 0; j < (int)blockSize; ++j) {
            if (j >= i) {
                acc += _pSamples[j+start] * _pSamples[j-i+start];
            }
        }
        // smoothed update
        _R[i] += alpha*(acc - _R[i]);
    }

    // calculate new filter coefficients
    // Durbin's recursion, per p. 411 of Rabiner & Schafer 1978
    E = _R[0];
    for (i = 1; i <= _p; ++i) {
        float sumalphaR = 0;
        for (j = 1; j < i; ++j) {
            sumalphaR += _ai[j]*_R[i-j];
        }
        ki = (_R[i] - sumalphaR)/E;
        _ai[i] = ki;
        for (j = 1; j <= i/2; ++j) {
            float aj = _ai[j];
            float aimj = _ai[i-j];
            _ai[j] = aj - ki*aimj;
            _ai[i-j] = aimj - ki*aj;
        }
        E = (1-ki*ki)*E;
    }
    // calculate new output
    for (i = 0; i < (int)blockSize; ++i) {
        float acc = _pSamples[i+start];
        for (j = 1; j <= _p; ++j) {
            if (i-j < 0) {
                acc -= _ai[j]*_Xo[_p + i-j];
            } else {
                acc -= _ai[j]*_pSamples[i-j+start];
            }
        }
        _whitened[i+start] = acc;
    }
    // save last few frames of input
    for (i = 0; i <= _p; ++i) {
        _Xo[i] = _pSamples[blockSize-1-_p+i+start];
    }
}


