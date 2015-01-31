# liblastfm Clementine version

* Add ws::Server attribute to override lastfm host.
* Disable some unnecessary options (for Clementine) in CMakeList.
* Rename src directory to lastfm.
* Remove deprecated warnings.
* Always use Qt4.

# liblastfm

liblastfm is a collection of libraries to help you integrate Last.fm services
into your rich desktop software. It is officially supported software developed
by Last.fm staff.

Michael Coffey http://twitter.com/eartle

Fork it: http://github.com/lastfm/liblastfm

Join us for chats on IRC!

Server: irc.last.fm
Channel: #last.desktop

# Dependencies

liblastfm requires:

* Qt 4.8 http://qt.digia.com/
* FFTW http://www.fftw.org/
* libsamplerate http://www.mega-nerd.com/SRC/

Additionally, to build it you will need:

* cmake

## Mac OS X

We recommend that you use Homebrew to install dependancies http://mxcl.github.com/homebrew/

    brew install qt

## Linux/*NIX

Do something like this:

    sudo apt-get install qt4-qmake pkg-config g++ libqt4-dev cmake libfftw-dev libsamplerate0-dev

Please note, we have only tested on Linux, but we think it'll work on all
varieties of UNIX. If it doesn't, report the bug to eartle on GitHub.

## Windows

Install Visual Studio 2008 or higher. Install Qt. Install the
Windows Server 2003 Platform SDK r2:

http://www.microsoft.com/Downloads/details.aspx?FamilyID=484269e2-3b89-47e3-8eb7-1f2be6d7123a

Set up your environment variables so all include paths and tools are
available.

Open a plain Windows shell, and see the next section.

# Installing liblastfm

    mkdir _build && cd _build
    cmake ..
    make -j4
    sudo make install

# Using liblastfm

We have copied the API at http://last.fm/api onto C++, so like you find
artist.getInfo there you will find an lastfm::Artist::getInfo function in our
C++ API. lastfm is a namespace, Artist a class and getInfo a function.

Thus the API is quite easy to learn. We suggest installing and checking the
include/lastfm/* directory to find out all capabilities.

The demos directory shows some further basic usage including Audioscrobbling
and getting metadata for music via our fingerprinting technology.

You need an API key from http://last.fm/api to use the webservice API.

Your link line needs to include the following:

    -llastfm -lQtCore -lQtNetwork -lQtXml

## Radio

Please set an identifiable UserAgent on your HTTP requests for the actual MP3s,
in extreme cases we'll contact you directly and demand you do so :P

## HTTP & Networking

You can specify your own QNetworkAccessManager derived class for liblastfm to
use with lastfm::setNetworkAccessManager(). Our default is pretty good
though, auto-determining proxy settings on Windows and OS X for instance.

# Development

## Public Headers

1. Header guards should be prefixed with LASTFM, eg. LASTFM_WS_REPLY_H
2. #includes should be to the system path eg. #include <lastfm/Scrobbler.h>
3. Don't make a header public unless it is absolutely required!
4. All headers under src/ (not subfolders) are public by default
