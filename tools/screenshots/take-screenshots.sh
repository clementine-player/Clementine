#!/bin/sh
# Takes the screenshots with a throwaway profile on Linux, so it can't touch
# your library or settings, or hand off to a Clementine you're running. The
# Test job in .github/workflows/all.yml runs it; the Windows and macOS jobs run
# clementine --screenshots directly, on runners nobody else uses.
#
#   take-screenshots.sh <clementine binary> <music dir> <screenshots dir>
#
# The music is generate-music.sh's. Without a display, set
# QT_QPA_PLATFORM=offscreen.
set -eu

binary=$1
music=$2
out=$3

profile=$(mktemp -d)
# Short, because Qt's local sockets live here and Unix socket paths are
# limited to about 100 characters: longer, and the tag reader never starts.
sockets=$(mktemp -d /tmp/cst-XXXXXX)
trap 'rm -rf "$profile" "$sockets"' EXIT

HOME=$profile \
XDG_CONFIG_HOME=$profile/.config \
XDG_DATA_HOME=$profile/.local/share \
XDG_CACHE_HOME=$profile/.cache \
TMPDIR=$sockets \
DBUS_SESSION_BUS_ADDRESS=disabled: \
  "$binary" --screenshots "$out" "$music"
