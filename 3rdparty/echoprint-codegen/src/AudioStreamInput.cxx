//
//  echoprint-codegen
//  Copyright 2011 The Echo Nest Corporation. All rights reserved.
//



#include <stddef.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#ifndef _WIN32
#include <unistd.h>
#define POPEN_MODE "r"
#else
#include "win_unistd.h"
#include <winsock.h>
#define POPEN_MODE "rb"
#endif
#include <string.h>

#include "AudioStreamInput.h"
#include "Common.h"
#include "Params.h"

using std::string;

namespace FFMPEG {
    // Do we think FFmpeg will read this as an audio file?
    bool IsAudioFile(const char* pFileName) {
        static const char* supportedExtensions[] = {".mp3", ".m4a", ".mp4", ".aif", ".aiff", ".flac", ".au", ".wav", ".aac", ".flv"};
        // Not an exhaustive list. ogg and rm could be added if tested.
        for (uint i = 0; i < NELEM(supportedExtensions); i++) {
            if (File::ends_with(pFileName, supportedExtensions[i]))
                return true;
        }
        return false;
    }
}

bool AudioStreamInput::IsSupported(const char *path) {
    return true; // Take a crack at anything, by default. The worst thing that will happen is that we fail.
}

AudioStreamInput::AudioStreamInput() : _pSamples(NULL), _NumberSamples(0), _Offset_s(0), _Seconds(0) {}

AudioStreamInput::~AudioStreamInput() {
    if (_pSamples != NULL)
        delete [] _pSamples, _pSamples = NULL;
}


bool AudioStreamInput::ProcessFile(const char* filename, int offset_s/*=0*/, int seconds/*=0*/) {
    if (!File::Exists(filename) || !IsSupported(filename))
        return false;

    _Offset_s = offset_s;
    _Seconds = seconds;
    std::string message = GetCommandLine(filename);

    FILE* fp = popen(message.c_str(), POPEN_MODE);
    bool ok = (fp != NULL);
    if (ok)
    {
        bool did_work = ProcessFilePointer(fp);
        bool succeeded = !pclose(fp);
        ok = did_work && succeeded;
    }
    else
        fprintf(stderr, "AudioStreamInput::ProcessFile can't open %s\n", filename);

    return ok;
}

// reads raw signed 16-bit shorts from a file
bool AudioStreamInput::ProcessRawFile(const char* rawFilename) {
    FILE* fp = fopen(rawFilename, "r"); // TODO: Windows
    bool ok = (fp != NULL);
    if (ok)
    {
        ok = ProcessFilePointer(fp);
        fclose(fp);
    }

    return ok;
}

// reads raw signed 16-bit shorts from stdin, for example:
// ffmpeg -i fille.mp3 -f s16le -ac 1 -ar 11025 - | TestAudioSTreamInput
bool AudioStreamInput::ProcessStandardInput(void) {
    // TODO - Windows will explodey at not setting O_BINARY on stdin.
    return ProcessFilePointer(stdin);
}

bool AudioStreamInput::ProcessFilePointer(FILE* pFile) {
    std::vector<short*> vChunks;
    uint nSamplesPerChunk = (uint) Params::AudioStreamInput::SamplingRate * Params::AudioStreamInput::SecondsPerChunk;
    uint samplesRead = 0;
    do {
        short* pChunk = new short[nSamplesPerChunk];
        samplesRead = fread(pChunk, sizeof (short), nSamplesPerChunk, pFile);
        _NumberSamples += samplesRead;
        vChunks.push_back(pChunk);
    } while (samplesRead > 0);

    // Convert from shorts to 16-bit floats and copy into sample buffer.
    uint sampleCounter = 0;
    _pSamples = new float[_NumberSamples];
    uint samplesLeft = _NumberSamples;
    for (uint i = 0; i < vChunks.size(); i++)
    {
        short* pChunk = vChunks[i];
        uint numSamples = samplesLeft < nSamplesPerChunk ? samplesLeft : nSamplesPerChunk;

        for (uint j = 0; j < numSamples; j++)
            _pSamples[sampleCounter++] = (float) pChunk[j] / 32768.0f;

        samplesLeft -= numSamples;
        delete [] pChunk, vChunks[i] = NULL;
    }
    assert(samplesLeft == 0);

    int error = ferror(pFile);
    bool success = error == 0;

    if (!success)
        perror("ProcessFilePointer error");
    return success;
}


