# Codegen for Echoprint

Echoprint is an open source music fingerprint and resolving framework powered by the [The Echo Nest](http://the.echonest.com/ "The Echo Nest"). The [code generator](http://github.com/echonest/echoprint-codegen "echoprint-codegen") (library to convert PCM samples from a microphone or file into Echoprint codes) is open source (MIT licensed) and free for any use. The [server component](http://github.com/echonest/echoprint-server "echoprint-server") that stores and resolves queries is open source (Apache 2 licensed) and free for any use. The [data for resolving to millions of songs](http://echoprint.me/data "Echoprint Data") is free for any use provided any changes or additions are merged back to the community. 

[Read more about Echoprint here](http://echoprint.me)

There are two modes of operation of the Echoprint codegen:

1. the codegen library (libcodegen) is meant to be linked into code that passes it a buffer of PCM data and will output a code string.
 
2. the codegen binary runs standalone, accepts filenames as inputs and runs in a multithreaded worker mode.

## Requirements for libcodegen

* Boost >= 1.35

## Additional requirements for the codegen binary

* [TagLib](http://developer.kde.org/~wheeler/taglib.html "TagLib")
* ffmpeg - this is called via shell and is not linked into codegen

## Notes about libcodegen:

Code generation takes a buffer of floating point PCM data sampled at 11025 Hz and mono. 

    Codegen * pCodegen = new Codegen(const float* pcm, uint numSamples, int start_offset);

    pcm: a buffer of floats, mono, 11025 Hz
    numSamples: the number of samples
    start_offset: creates a hint to the server on where the sample is taken from in the original file if known

    string code = pCodegen->getCodeString(); 

The code string is just a base64 encoding of a zlib compression of the original code string, which is a hex encoded series of ASCII numbers. See API/fp.py in echoprint-server for decoding help.

You only need to query for 20 seconds of audio to get a result.

## Notes about the codegen binary

The makefile builds an example code generator that uses libcodegen, called "codegen." This code generator has more features -- it will output ID3 tag information and uses ffmpeg to decode any type of file. If you don't need to compile libcodegen into your app you can rely on this. Note that you need to have ffmpeg installed and accessible on your path for this to work.

    ./echoprint-codegen billie_jean.mp3 10 30

Will take 30 seconds of audio from 10 seconds into the file and output JSON suitable for querying:

    {"metadata":{"artist":"Michael jackson", "release":"800 chansons des annes 80", "title":"Billie jean", "genre":"", "bitrate":192, "sample_rate":44100, "seconds":294, "filename":"billie_jean.mp3", "samples_decoded":220598, "given_duration":30, "start_offset":10, "version":4.00}, "code_count":846, "code":"JxVlIuNwzAMQ1fxCDL133+xo1rnGqNAEcWy/ERa2aKeZmW...

You can POST this JSON directly to the Echo Nest's [song/identify](http://developer.echonest.com/docs/v4/song.html#identify "song/identify") (who has an Echoprint server booted), for example:

    curl -F "query=@post_string" http://developer.echonest.com/api/v4/song/identify?api_key=YOUR_KEY
    {"fp_lookup_time_ms": 21, "results": [{"songID": "SOAFVGQ1280ED4E371", "match_type": "fp", "title": "Billie Jean", "artist": "Michael Jackson", "artistID": "ARXPPEY1187FB51DF4", "score": 63, "release": "Thriller"}]
    (you can also use GET, see the API description)

Or you can host your own [Echoprint server](http://github.com/echonest/echoprint-server "echoprint-server") and ingest or query to that.

Codegen also runs in a multithreaded mode for bulk resolving:

    ./echoprint-codegen -s 10 30 < file_list

Will compute codes for every file in file_list for 30 seconds starting at 10 seconds. (It tries to be smart about the number of threads to use.) It will output a JSON list. Note that song/identify can accept lists in the JSON, which will be faster than sending each code one at a time. The "tag" parameter is added to each code dictionary to match the resolving material.

## Statistics

### Speed

Codegen scans audio at roughly 250x real time per processor after decoding and resampling to 11025 Hz. This means a full song can be scanned in less than 0.5s on an average computer, and an amount of audio suitable for querying (30s) can be scanned in less than 0.04s.

Decoding from MP3 will be the bottleneck for most implementations. Decoders like mpg123 or ffmpeg can decode 30s mp3 audio to 11025 PCM in under 0.10s.

    clump:echoprint-codegen bwhitman$ time mpg123 -q -s -4 -n 1200 song.mp3  > /dev/null
    real        0m0.079s
    user        0m0.067s
    sys         0m0.007s

### Accuracy

Look at http://echoprint.me for information on the accuracy of the echoprint system.

## FAQ

Q: I get "Couldn't decode any samples with: ffmpeg" when running codegen

A: When running the example code generator (echoprint-codegen) make sure ffmpeg is accessible to your path. Try running ffmpeg filename.mp3 on the file you are testing the code generator with. If it doesn't work, codegen won't work.
