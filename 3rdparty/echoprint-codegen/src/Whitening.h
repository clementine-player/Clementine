//
//  echoprint-codegen
//  Copyright 2011 The Echo Nest Corporation. All rights reserved.
//


#ifndef WHITENING_H
#define WHITENING_H
#include "Common.h"
#include "Params.h"
#include "MatrixUtility.h"


class AudioStreamInput;

class Whitening {
public:
    inline Whitening() {};
    Whitening(AudioStreamInput* pAudio);
    Whitening(const float* pSamples, uint numSamples);
    virtual ~Whitening();
    void Compute();
    void ComputeBlock(int start, int blockSize);

public:
    float* getWhitenedSamples() const {return _whitened;}
    inline uint getNumSamples() const {return _NumSamples;}

protected:
    const float* _pSamples;
    float* _whitened;
    uint _NumSamples;
    float* _R;
    float *_Xo;
    float *_ai;
    int _p;
private:
    void Init();
};

#endif
