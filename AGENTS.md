# Clementine

## Commit messages

A commit that changes something users will notice ends with a `Release-note:`
trailer: one line, written for users (what's new, not how it was done). The
weekly release collects them for its release notes, and a week with none
makes no release. Commits without user-visible changes (refactoring, tests,
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

## C++ style

Follow the [Google C++ style guide](https://google.github.io/styleguide/cppguide.html),
except where it conflicts with Qt's style or the code around it, which wins.
`dist/format.py` formats with `.clang-format`, which is based on Google's.

One rule to note: don't hold or pass mutable references. Use a pointer for
something that's changed, and a `const` reference otherwise:

    Client* client = &clients_[client_id];  // not Client& client = ...
    void Manifest(const Request& req, Response* response);

Operators that take a stream, such as `operator<<(std::ostream&, ...)`, are
the usual exception.

## Stacked pull requests

A pull request that builds on another one targets that one's branch, so that
it shows only its own commits. Its description starts by naming the pull
requests it depends on. CI runs on pull requests into any branch, so it's
checked against what it builds on.

Pull requests are merged by squashing or rebasing, which gives their commits
new hashes, so the branches built on them still carry the old ones. When one
merges, rebase the pull requests that build on it straight away, moving only
their own commits:

    git rebase --onto origin/master <the merged branch's last commit> <branch>

Keep the branches linear: don't merge `master` or another branch into one
instead, because GitHub can't rebase a branch with merge commits.

`git patch-id --stable` on the branch's commit and the merged one shows
whether what merged is what the branch had. Then build, run the tests, push
with `--force-with-lease`, point each at `master` if GitHub hasn't already
(`gh pr edit <number> --base master`), and update its description.
