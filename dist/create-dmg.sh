#!/bin/sh
# author: max@last.fm, muesli@tomahawk-player.org
# brief:  Produces a compressed DMG from a bundle directory
# usage:  Pass the bundle directory as the only parameter
# note:   This script depends on the Tomahawk build system, and must be run from
#         the build directory
################################################################################

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

# clean up
rm -rf "$TMP"
rm -f "$OUT"

# create DMG contents and copy files
mkdir -p "$TMP/.background"
cp ../dist/dmg_background.png "$TMP/.background/background.png"
cp ../dist/DS_Store.in "$TMP/.DS_Store"
chmod go-rwx "$TMP/.DS_Store"
ln -s /Applications "$TMP/Applications"
# Copies the prepared bundle into the dir that will become the DMG, renamed
# to "Clementine.app" (capital C, matching DS_Store.in's icon layout) with
# its extension hidden in Finder - otherwise Finder shows the on-disk name
# "clementine.app" verbatim, which overflows its icon label in the DMG
# window.
cp -R "$IN" "$TMP/Clementine.app"
SetFile -a E "$TMP/Clementine.app"

# create
# APFS, not HFS+ (the old `hdiutil makehybrid -hfs` approach this replaced):
# HFS+ volumes carry a native per-file Finder-info catalog field that gets
# exposed as a com.apple.FinderInfo xattr on every file, and that attribute
# survives copying out of the mounted image - which fails `codesign
# --strict` (and therefore Gatekeeper) on the signed app inside. APFS
# doesn't have this legacy baggage, and this is also just a single
# `hdiutil create` call instead of the old makehybrid+convert two-step
# hybrid-CD-era dance.
hdiutil create -volname Clementine -srcfolder "$TMP" -ov -format UDZO -imagekey zlib-level=9 -fs APFS "$OUT"

# done !
echo 'DMG size:' `du -hs "$OUT" | awk '{print $1}'`
