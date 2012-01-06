Chromaprint
===========

Dependencies
------------

The library itself only depends on a FFT library, which at the moment can
be either FFmpeg [1] (at least r22291, 0.6 is fine), FFTW3 [2] or if you are
on iOS or OS X, you can use the Accelerate/vDSP framework. See the next
section for details.

The tools included in the package require FFmpeg (can be older), TagLib [3]
and Boost Filesystem [4].

In order to build the test suite, you will need the Google Test library [5].

[1] http://www.ffmpeg.org/
[2] http://www.fftw.org/
[3] http://developer.kde.org/~wheeler/taglib.html
[4] http://www.boost.org/
[5] http://code.google.com/p/googletest/

Installing
----------

The most common way to build Chromaprint is like this:

$ cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_EXAMPLES=ON .
$ make
$ sudo make install

This will build Chromaprint as a shared library and also include the fpcalc
utility (which is used by MusicBrainz Picard, for example).

See below for other options.

FFT Library
-----------

Chromaprint can use three FFT libraries, FFmpeg, FFTW3 and vDSP. FFmpeg is
preffered, as it's a little faster for our purposes and it's LGPL-licensed,
so it doesn't impact the license of Chromaprint. The FFT interface was added
only recently though, so it might not be available in Linux distributions yet.
FFTW3 can be used in this case, but this library is released under the GPL
license, which makes also the resulting Chromaprint binary GPL licensed.

If you run simple `cmake .`, it will try to find both FFmpeg and FFTW3 and
select the first one it finds. If you have new FFmpeg installed in a separate
location, you can let CMake know using the `FFMPEG_ROOT` option:

$ cmake -DFFMPEG_ROOT=/path/to/local/ffmpeg/install .

If you have new FFmpeg installed, but for some reason prefer to use FFTW3, you
can use the `WITH_FFTW3` option:  

$ cmake -DWITH_FFTW3=ON .

There is also a `WITH_AVFFT` option, but the script will select the FFmpeg FFT
automatically if it's available, so it shouldn't be necessary to use it.

If you are on Mac, you can use the standard Accelerate framework with the vDSP
library. This requires you to install no external libraries. It will use
vDSP by default on OS X (but there still is a `WITH_VDSP` option).

Unit Tests
----------

The test suite can be built and run using the following commands:

$ cmake -DBUILD_TESTS=ON .
$ make check

Related Projects
----------------

 * pyacoustid - https://github.com/sampsyo/pyacoustid
 * gst-chromaprint - https://github.com/lalinsky/gst-chromaprint

Standing on the Shoulder of Giants
----------------------------------

I've learned a lot while working on this project, which would not be possible
without having information from past research. I've read many papers, but the
concrete ideas implemented in this library are based on the following papers:

 * Yan Ke, Derek Hoiem, Rahul Sukthankar. Computer Vision for Music
   Identification, Proceedings of Computer Vision and Pattern Recognition,
   2005. http://www.cs.cmu.edu/~yke/musicretrieval/

 * Frank Kurth, Meinard Müller. Efficient Index-Based Audio Matching, 2008.
   http://dx.doi.org/10.1109/TASL.2007.911552

 * Dalwon Jang, Chang D. Yoo, Sunil Lee, Sungwoong Kim, Ton Kalker.
   Pairwise Boosted Audio Fingerprint, 2009.
   http://dx.doi.org/10.1109/TIFS.2009.2034452

