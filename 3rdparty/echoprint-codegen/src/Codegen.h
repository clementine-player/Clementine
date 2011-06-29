//
//  echoprint-codegen
//  Copyright 2011 The Echo Nest Corporation. All rights reserved.
//


#ifndef CODEGEN_H
#define CODEGEN_H

// Entry point for generating codes from PCM data.

#include <string>
#include <vector>
#include <sys/types.h>

#ifdef _MSC_VER
    #ifdef CODEGEN_EXPORTS
        #define CODEGEN_API __declspec(dllexport)
        #pragma message("Exporting codegen.dll")
    #else
        #define CODEGEN_API __declspec(dllimport)
        #pragma message("Importing codegen.dll")
    #endif
#else
    #define CODEGEN_API
#endif

class Fingerprint;
class SubbandAnalysis;
struct FPCode;

class CODEGEN_API Codegen {
public:
    Codegen(const float* pcm, unsigned int numSamples, int start_offset);

    std::string getCodeString(){return _CodeString;}
    int getNumCodes(){return _NumCodes;}
    static float getVersion();
private:
    Fingerprint* computeFingerprint(SubbandAnalysis *pSubbandAnalysis, int start_offset);
    std::string createCodeString(std::vector<FPCode> vCodes);

    std::string compress(const std::string& s);
    std::string _CodeString;
    int _NumCodes;
};

#endif
