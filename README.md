Clementine ![all](https://github.com/clementine-player/Clementine/workflows/all/badge.svg)
----------------

Clementine is a modern music player and library organizer for Windows, Linux and macOS.

- [Latest Release](https://github.com/clementine-player/Clementine/releases/latest)
- [Latest Pre-Releases](https://github.com/clementine-player/Clementine/releases)
- Website: http://www.clementine-player.org/
- Github: https://github.com/clementine-player/Clementine

Opening an issue
----------------
### Ask for a new feature

Please:

 * Check if the new feature is not already implemented (Changelog)
 * Check if another person didn't already open an issue
 * If there is already an opened issue there is no need to comment "+1", it won't help. Instead, you can subscribe to the issue to be notified of anything new about it

### Report a bug

Please:
 
 * Try the latest build (https://github.com/clementine-player/Clementine/releases) to see if any bug is still present. If it works fine even though you see an open issue, please comment on it and explain that the issue has been fixed.
 * Check if another person has already opened the same issue to avoid duplicates
 * If there already is an open issue you could comment on it to add detail about the problem or confirm it
 * In case there isn't, you can open a new issue with an explicit title and as much information as possible (OS, Clementine version, how to reproduce the problem...)
 * Please use https://pastebin.com/ for logs/debug.
 
If there are no answers, it doesn't mean we don't care about your feature request/bug. It just means we can't reproduce the bug or haven't had time to implement it :o)

Compiling from source
---------------------

Get the code (if you haven't already):

    git clone https://github.com/clementine-player/Clementine.git && cd Clementine

Compile and install:

    cd bin
    cmake ..
    make -j$(nproc)
    sudo make install

See the Wiki for more instructions and a list of dependencies:
https://github.com/clementine-player/Clementine/wiki#compiling-and-installing-clementine
