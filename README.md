Clementine
==========

This is my edited version of Clementine. My aim is to make it more stable and less precarious. I'm intending to use it locally, so I've disabled all online music services.

A Clementine fork:
------------------
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
