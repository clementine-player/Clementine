Clementine
==========

Clementine is a modern music player and library organizer for Windows, Linux and Mac OS X.

- Website: http://www.clementine-player.org/
- Github: https://github.com/clementine-player/Clementine
- Buildbot: http://buildbot.clementine-player.org/grid
- Latest developer builds: http://builds.clementine-player.org/

Opening an issue
----------------
### Ask for a new feature

Please:

 * Check if the new feature is not already implemented (Changelog)
 * Check if another person didn't already open an issue
 * If there is already an opened issue there is no need to comment "+1", it won't help. Instead, you can subscribe to the issue to be notified of anything new about it

### Report a bug

Please:
 
 * Try with the latest developer build (http://builds.clementine-player.org/) to see if the bug is still present (**Attention**, those builds aren't stable so they might not work well and could sometimes break things like user settings). If if works like a charm although you see an opened issue please comment it to explain the issue was fixed
 * Check if another person didn't already open an issue
 * If there is already an opened issue, you can try to comment it to add precisions about the problem or confirm it
 * If there isn't, you can open a new issue with an explicit title and much information as possible (OS, Clementine version, what steps will reproduce the problem...)
 * Please use http://pastebin.com/ for logs/debug
 
If there is no answers, it doesn't mean we don't care about your feature request/bug, it just mean we can't reproduce the bug or haven't time to implement it :o)

Compiling from source
---------------------

Get the code (if you haven't already):

    git clone https://github.com/clementine-player/Clementine.git && cd Clementine

Compile and install:

    cd bin
    cmake ..
    make -j8
    sudo make install

See the Wiki for more instructions and a list of dependencies:
https://github.com/clementine-player/Clementine/wiki/Compiling-from-Source
