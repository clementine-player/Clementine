//
//  echoprint-codegen
//  Copyright 2011 The Echo Nest Corporation. All rights reserved.
//



#ifndef METADATA_H
#define METADATA_H


#include <string>
using std::string;

class Metadata {
public:
    Metadata(const std::string& file);
    string Filename() {return _Filename;}
    string Artist(){ return _Artist;}
    string Album() { return _Album;}
    string Title() { return _Title;}
    string Genre() { return _Genre;}
    int Bitrate()  { return _Bitrate;}
    int SampleRate(){ return _SampleRate;}
    int Seconds() { return _Seconds;}
private:
    string _Filename;
    string _Artist;
    string _Album;
    string _Title;
    string _Genre;

    int _Bitrate;
    int _SampleRate;
    int _Seconds;
};
#endif

