#!/bin/sh -e

REFSPEC=$1
DIST=$2

if [ -z "$REFSPEC" -o -z "$DIST" ]; then
  echo "Usage: $0 <refspec> <dist>"
  echo "Example: $0 tags/0.7 natty"
  exit 1
fi

PPA=ppa:me-davidsansome/clementine
REPO=https://code.google.com/p/clementine-player/

BASE=`pwd`
DIRECTORY=clementine

# Cleanup any old stuff
rm -rfv $BASE/$DIRECTORY $BASE/*.diff.gz $BASE/*.tar.gz $BASE/*.dsc $BASE/*_source.changes

# Checkout
git clone $REPO $DIRECTORY
cd $BASE/$DIRECTORY
git checkout $REFSPEC

# Generate changelog and maketarball.sh
mkdir $BASE/$DIRECTORY/bin
cd $BASE/$DIRECTORY/bin
cmake .. -DDEB_DIST=$DIST -DWITH_DEBIAN=ON
rm -rfv $BASE/$DIRECTORY/bin/*

# Create the tarball
cd $BASE/$DIRECTORY/dist
./maketarball.sh
mv -v $BASE/$DIRECTORY/dist/*.orig.tar.gz $BASE/
rm -v $BASE/$DIRECTORY/dist/*.tar.gz
rm -vrf $BASE/$DIRECTORY/.git

# Build the deb
cd $BASE/$DIRECTORY
dpkg-buildpackage -S -kF6ABD82E

# Upload to ppa
cd $BASE
dput $PPA *_source.changes
