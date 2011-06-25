//
//  echoprint-codegen
//  Copyright 2011 The Echo Nest Corporation. All rights reserved.
//



#ifndef PARAMS_H
#define PARAMS_H

#include "Common.h"

namespace Params {
    namespace AudioStreamInput {
        const float SamplingRate = 11025.0f;
        const uint Channels = 1;
        const float SecondsPerChunk = 10.0f;
        const uint BytesPerSample = 4; // floats
        const uint MaxSamples = 66977792; // TODO: Remove this, or set it intelligently, at least. Good for 32-bit analyzer.
    }
}

#endif
