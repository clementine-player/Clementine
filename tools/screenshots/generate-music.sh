#!/bin/sh
# Generates the showcase library the screenshots show, from showcase-library.tsv:
# tagged Ogg Vorbis tracks of sine tones, with each album's cover beside its
# tracks. The Android and iOS remotes' clementine-it/ has the same library, so
# every Clementine's screenshots show the same music.
#
#   generate-music.sh <dir>
#
# Uses GStreamer rather than ffmpeg, as the remotes do, because every job that
# builds Clementine already has it.
set -eu

out=$1
here=$(dirname "$0")
n=0
t=0
last_album=
grep -v '^#' "$here/showcase-library.tsv" |
while IFS="$(printf '\t')" read -r artist album year title seconds; do
  n=$((n + 1))
  [ "$album" = "$last_album" ] || t=0
  last_album=$album
  t=$((t + 1))
  dir="$out/$artist/$album"
  file="$dir/$(printf %02d "$t") $title.ogg"
  mkdir -p "$dir"
  # Where Clementine looks for a cover: an image in the album's folder.
  cover="$here/covers/$album.jpg"
  if [ -f "$cover" ]; then cp "$cover" "$dir/cover.jpg"; fi
  # gst-launch joins its arguments and parses them again, so a property with
  # spaces in it is quoted, and the quotes inside it escaped.
  q='\"'
  tags="artist=$q$artist$q,album-artist=$q$artist$q,composer=$q$artist$q"
  tags="$tags,album=$q$album$q,title=$q$title$q,track-number=(uint)$t"
  tags="$tags,datetime=(datetime)$year,genre=Classical"
  # One second a buffer, at a rate low enough to keep the files small.
  gst-launch-1.0 -q \
    audiotestsrc wave=sine freq=$((220 + n * 55)) num-buffers="$seconds" \
      samplesperbuffer=8000 ! audio/x-raw,rate=8000,channels=1 ! \
    audioconvert ! taginject "tags=\"$tags\"" ! vorbisenc ! oggmux ! \
    filesink location="$file" < /dev/null
done
find "$out" -name '*.ogg' | sort
