//
//  echoprint-codegen
//  Copyright 2011 The Echo Nest Corporation. All rights reserved.
//


#include "Metadata.h"
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <iostream>

Metadata::Metadata(const string& file) : _Filename(file), _Artist(""), _Album(""), _Title(""), _Genre(""), _Bitrate(0), _SampleRate(0), _Seconds(0) {
    if (file != "stdin") {
        // TODO: Consider removing the path from the filename -- not sure if we can do this in a platform-independent way.
        TagLib::FileRef f(_Filename.c_str());

        TagLib::Tag* tag = f.isNull() ? NULL : f.tag();
        if (tag != NULL) {
            _Artist = tag->artist().toCString();
            _Album = tag->album().toCString();
            _Title = tag->title().toCString();
            _Genre = tag->genre().toCString();
        }

        TagLib::AudioProperties* properties = f.isNull() ? NULL : f.audioProperties();
        if (properties != NULL) {
            _Bitrate = properties->bitrate();
            _SampleRate = properties->sampleRate();
            _Seconds = properties->length();
        }
    }
}

