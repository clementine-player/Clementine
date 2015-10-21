#!/bin/bash


#
# Builds the debian package of clementine.
# Modify arch and dist variables if needed.
#
# Note: this is just a helper script to help
# install all dependencies and build clementine
# with one command. Do proper debian packaging
# elsewhere.
#

sudo apt-get install devscripts equivs
sudo mk-build-deps --install ../debian/control
mkdir -p ../bin
cmake ../ -DWITH_DEBIAN=ON -DDEB_ARCH=amd64 -DDEB_DIST=ubuntu -DENABLE_SPOTIFY_BLOB=OFF
make deb

