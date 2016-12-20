#!/bin/sh
# author: max@last.fm, muesli@tomahawk-player.org
# brief:  Produces a compressed DMG from a bundle directory
# usage:  Pass the bundle directory as the only parameter
# note:   This script depends on the Tomahawk build system, and must be run from
#         the build directory
################################################################################


#if [ -z $VERSION ]
#then
#    echo VERSION must be set
#    exit 2
#fi

if [ -z "$1" ]
then
    echo "Please pass the bundle.app directory as the first parameter."
    exit 3
fi
################################################################################


NAME=$(basename "$1" | perl -pe 's/(.*).app/\1/')
IN="$1"
TMP="dmg/$NAME"
OUT="$NAME.dmg"
mkdir -p "$TMP"
################################################################################

genisoimage -D -V "Clementine" -no-pad -r -apple -o $NAME.iso $IN
dmg dmg $NAME.iso $OUT
