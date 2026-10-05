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

## Stacked pull requests

CI only runs on pull requests into `master`, so a pull request that builds on
another one still targets `master`. Its description starts by naming the pull
requests it depends on, and which of its commits to review until they merge.

Pull requests are merged by squashing or rebasing, which gives their commits
new hashes, so the branches built on them still carry the old ones. When one
merges, rebase the pull requests that build on it straight away, moving only
their own commits:

    git rebase --onto origin/master <the merged branch's last commit> <branch>

`git patch-id --stable` on the branch's commit and the merged one shows
whether what merged is what the branch had. Then build, run the tests, push
with `--force-with-lease`, and update the commits to review in each
description.
