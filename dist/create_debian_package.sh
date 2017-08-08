#!/bin/bash


#
# Builds the debian package of clementine.
# Modify arch and dist variables if needed.
#

mkdir -p ../bin
cmake ../ -DWITH_DEBIAN=ON -DDEB_ARCH=amd64 -DDEB_DIST=ubuntu -DENABLE_SPOTIFY_BLOB=OFF
make deb

echo
echo "Clementine should now be built in a deb package above."
echo
echo "If the build failed, you might want to run ./install_debian_dependencies.sh"
echo "to install any missing dependency packages."
echo

