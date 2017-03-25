Clementine
==========

This is the edited version of Clementine. I've tweaked a few things to make it more stable, more local and less precarious.

This is a list of changes (updated over time):
* Don't build any of the external services.
* Disable playlist sorting according to columns (as there is no way to bring back original sorting).

A Clementine fork:
- Website: http://www.clementine-player.org/
- Github: https://github.com/clementine-player/Clementine
- Buildbot: http://buildbot.clementine-player.org/grid
- Latest developer builds: http://builds.clementine-player.org/

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
