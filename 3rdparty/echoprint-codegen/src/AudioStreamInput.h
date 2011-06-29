//
//  echoprint-codegen
//  Copyright 2011 The Echo Nest Corporation. All rights reserved.
//


#ifndef AUDIOSTREAMINPUT_H
#define AUDIOSTREAMINPUT_H
#include "Common.h"
#include "Params.h"
#include <iostream>
#include <string>
#include <math.h>
#include "File.h"
#if defined(_WIN32) && !defined(__MINGW32__)
#define and &&
#define snprintf _snprintf
#endif

class AudioStreamInput {
public:
    AudioStreamInput();
    virtual ~AudioStreamInput(); 
    virtual bool ProcessFile(const char* filename, int offset_s=0, int seconds=0);
    virtual std::string GetName() = 0;
    bool ProcessRawFile(const char* rawFilename);
    bool ProcessStandardInput(void);
    bool ProcessFilePointer(FILE* pFile);
    int getNumSamples() const {return _NumberSamples;}
    const float* getSamples() {return _pSamples;} 
    double getDuration() { return (double)getNumSamples() / Params::AudioStreamInput::SamplingRate; }
    virtual bool IsSupported(const char* pFileName); //Everything ffmpeg can do, by default
    int GetOffset() const { return _Offset_s;}
    int GetSeconds() const { return _Seconds;}
protected:
    
    virtual std::string GetCommandLine(const char* filename) = 0;
    static bool ends_with(const char *s, const char *ends_with);
    float* _pSamples;
    uint _NumberSamples;
    int _Offset_s;
    int _Seconds;

};

class StdinStreamInput : public AudioStreamInput {
public:   
    std::string GetName(){return "stdin";};
protected:
    bool IsSupported(const char* pFileName){ return (std::string("stdin") == pFileName);};
    bool ProcessFile(const char* filename, int offset_s=0, int seconds=0){ return ProcessStandardInput();}
    virtual std::string GetCommandLine(const char* filename){return "";} // hack
};

class FfmpegStreamInput : public AudioStreamInput {
public:   
    std::string GetName(){return "ffmpeg";};
protected:
    std::string GetCommandLine(const char* filename) { 
        // TODO: Windows
        char message[4096] = {0};
        if (_Offset_s == 0 and _Seconds == 0)
            snprintf(message, NELEM(message), "ffmpeg -i \"%s\"  -ac %d -ar %d -f s16le - 2>/dev/null",
                    filename, Params::AudioStreamInput::Channels, (uint) Params::AudioStreamInput::SamplingRate);
        else
            snprintf(message, NELEM(message), "ffmpeg -i \"%s\"  -ac %d -ar %d -f s16le -t %d -ss %d - 2>/dev/null",
                    filename, Params::AudioStreamInput::Channels, (uint) Params::AudioStreamInput::SamplingRate, _Seconds, _Offset_s);
        
        printf("%s\n", message);
        return std::string(message);
    }
};

namespace FFMPEG {
    bool IsAudioFile(const char* pFileName);
};

class Mpg123StreamInput : public AudioStreamInput {
public:   
    std::string GetName(){return "mpg123";};
protected:
    #define FRAMES_PER_SECOND 38.2813f
    bool IsSupported(const char* pFileName){ return File::ends_with(pFileName, ".mp3");};
    std::string GetCommandLine(const char* filename) { 
        char message[4096] = {0};
        if (_Offset_s == 0 and _Seconds == 0)
            snprintf(message, NELEM(message), "mpg123 --quiet --singlemix --stdout --rate %d \"%s\"",
                (uint) Params::AudioStreamInput::SamplingRate, filename);
        else
            snprintf(message, NELEM(message), "mpg123 --quiet --singlemix --stdout --rate %d --skip %d --frames %d \"%s\"",
                (uint) Params::AudioStreamInput::SamplingRate, (uint)(_Offset_s * FRAMES_PER_SECOND) /* unprecise */, (uint)ceilf(_Seconds * FRAMES_PER_SECOND) /* unprecise */, filename);
        return std::string(message);
    }
};

#endif
