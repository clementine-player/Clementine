#!/usr/bin/env bash
# Decides whether there's a release to make, and what it is. Run by
# .github/workflows/release.yml, and runnable locally to see what the next
# release would be:
#
#   dist/plan_release.sh [notes file]
#
# A release is due when the commits since the last release (the newest
# numbered tag, such as 1.4.2, that the commit has) carry release notes: a
# "Release-note:" trailer in the commit message, one line for users. Commits
# without one (refactoring, tests, CI) don't make a release, unless ALWAYS is
# set (a release run by hand): then any commit since the last release makes
# one, with a general note if none has its own.
#
# The commit is COMMIT, or origin/master. The version is VERSION, if set, or
# the last release's with its last number one higher: 1.4.2, then 1.4.3. For
# a new minor or major version, run the release by hand with that version.
#
# Prints key=value lines (for $GITHUB_OUTPUT):
#   release  true, or false when there's nothing to release
#   version  the version, which release.yml tags the commit with
#   last     the previous release's tag
# and writes every note to the notes file (default: notes.md), one bullet
# each, for the GitHub release.
set -euo pipefail

cd "$(dirname "$0")/.."
commit=${COMMIT:-origin/master}
notes_file=${1:-notes.md}

# Numbered releases have annotated tags, which is all describe looks at
# without --tags: development builds' tags (1.4.2-55-g4c062729d) are
# lightweight.
last=$(git describe --abbrev=0 --match '[0-9]*' "$commit" 2>/dev/null || true)
echo "last=$last"

# The notes of every commit since then, oldest first, each once: the nightly
# translations commits all have the same one.
notes=$(git log --reverse --format='%(trailers:key=Release-note,valueonly,separator=%x0A)' \
  ${last:+"$last.."}"$commit" | sed -e '/^[[:space:]]*$/d' -e 's/^[[:space:]]*//' | awk '!seen[$0]++')
if [ -z "$notes" ] && [ -n "${ALWAYS:-}" ] && [ -n "$(git rev-list ${last:+"$last.."}"$commit")" ]; then
  notes="Fixes and improvements."
fi
if [ -z "$notes" ]; then
  echo "release=false"
  exit 0
fi

version=${VERSION:-}
if [ -z "$version" ]; then
  [ -n "$last" ] || { echo "::error::There's no release to number this one after: set VERSION." >&2; exit 1; }
  version="${last%.*}.$(( ${last##*.} + 1 ))"
  # 1.3 -> 1.3.1, rather than 1.4.
  [[ "$last" == *.*.* ]] || version="$last.1"
fi
[[ "$version" =~ ^[0-9]+(\.[0-9]+)+$ ]] || { echo "::error::$version isn't a version, such as 1.4.3." >&2; exit 1; }
if git rev-parse -q --verify "refs/tags/$version" > /dev/null; then
  echo "::error::There's a release $version already." >&2
  exit 1
fi

printf '%s\n' "$notes" | sed 's/^/- /' > "$notes_file"
echo "release=true"
echo "version=$version"
