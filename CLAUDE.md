# Clementine

## Commit messages

A commit that changes something users will notice ends with a `Release-note:`
trailer: one line, written for users (what's new, not how it was done). The
weekly macOS update collects them for its release notes, and a week with none
publishes no update. Commits without user-visible changes (refactoring, tests,
CI, docs) have no note. See "Release notes" in `README.md`.

Git only reads trailers in the message's last paragraph. Put `Release-note:`
in the same paragraph as `Co-Authored-By:` and any other trailers, with no
blank line between them; separated by a blank line, the note is silently
ignored:

    Fix the lyrics scrolling past the end of the song

    The position was compared with the song's length in the wrong units.

    Release-note: Lyrics no longer scroll past the end of the song.
    Co-Authored-By: Claude <noreply@anthropic.com>

After committing, check that git sees it:

    git log -1 --format='%(trailers:key=Release-note,valueonly)'
