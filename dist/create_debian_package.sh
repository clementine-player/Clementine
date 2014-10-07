#!/bin/bash


#
# Builds the debian package of clementine.
# Modify arch and dist variables if needed.
#


mkdir -p ../bin
cmake ../ -DWITH_DEBIAN=ON -DDEB_ARCH=amd64 -DDEB_DIST=ubuntu -DENABLE_SPOTIFY_BLOB=OFF
make deb

