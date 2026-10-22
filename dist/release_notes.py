#!/usr/bin/env python3
"""Collects Clementine's release notes, and plans the weekly release from them.

Release notes live in commit messages: a commit that changes something users
notice ends with a "Release-note:" trailer, one line for users (see "Release
notes" in README.md). There are two kinds of release, both tagged:

- Numbered releases, such as 1.4.3, made by .github/workflows/release.yml
  every week there are notes to tell users. Their tags are annotated.
- Development builds, such as 1.4.3-12-g4c062729d, made by all.yml for every
  push to master. Their tags are lightweight.

So `git describe` without --tags finds the last numbered release, and with
--tags the last build of either kind (see cmake/Version.cmake).

Usage:

  dist/release_notes.py plan [--commit C] [--version V] [--always] [--notes FILE]

    Whether there's a numbered release to make of commit C (default:
    origin/master), and its version. Run by release.yml, and locally to see
    what the next release would be. Prints key=value lines, for
    $GITHUB_OUTPUT: last (the last release), release (true or false) and
    version. Writes the notes to FILE (default: notes.md).

  dist/release_notes.py notes [--commit C] [--release]

    The GitHub release's notes for commit C (default: HEAD), which all.yml has
    just built: the notes since the build before it, or with --release, since
    the numbered release before it. Prints them as Markdown.
"""

import argparse
import os
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

# What a release notes when no commit since the last one has a note of its
# own: only when a release is made by hand.
GENERAL_NOTE = "Fixes and improvements."

# A numbered release's version: 1.4.3, or 1.5.
VERSION = re.compile(r"[0-9]+(\.[0-9]+)+")


class Error(Exception):
    pass


def git(*args: str) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        ["git", *args], cwd=ROOT, capture_output=True, text=True, check=False
    )


def git_output(*args: str) -> str:
    result = git(*args)
    if result.returncode != 0:
        raise Error(f"git {' '.join(args)} failed: {result.stderr.strip()}")
    return result.stdout


def last_release(commit: str, *, numbered: bool) -> str | None:
    """The newest release tag that commit has, if any.

    Numbered releases only, or development builds too.
    """
    args = ["describe", "--abbrev=0", "--match", "[0-9]*"]
    if not numbered:
        args.append("--tags")
    result = git(*args, commit)
    return result.stdout.strip() if result.returncode == 0 else None


def commits_since(last: str | None, commit: str) -> str:
    return f"{last}..{commit}" if last else commit


def notes_since(last: str | None, commit: str) -> list[str]:
    """The notes of the commits after last up to commit, oldest first.

    Each note only once: the nightly translations commits all have the same
    one.
    """
    log = git_output(
        "log",
        "--reverse",
        "--format=%(trailers:key=Release-note,valueonly,separator=%x0A)",
        commits_since(last, commit),
    )
    notes: list[str] = []
    for line in log.splitlines():
        note = line.strip()
        if note and note not in notes:
            notes.append(note)
    return notes


def has_commits_since(last: str | None, commit: str) -> bool:
    return bool(git_output("rev-list", "-n", "1", commits_since(last, commit)).strip())


def next_version(last: str) -> str:
    """The last release's version with its last number one higher.

    1.4.2 is followed by 1.4.3, and 1.3 by 1.3.1 rather than 1.4: a new minor
    or major version is made by hand.
    """
    numbers = last.split(".")
    if len(numbers) < 3:
        return f"{last}.1"
    numbers[-1] = str(int(numbers[-1]) + 1)
    return ".".join(numbers)


def as_markdown(notes: list[str]) -> str:
    return "".join(f"- {note}\n" for note in notes)


def plan(args: argparse.Namespace) -> None:
    last = last_release(args.commit, numbered=True)
    print(f"last={last or ''}")

    notes = notes_since(last, args.commit)
    # Run by hand, any change since the last release makes one.
    if not notes and args.always and has_commits_since(last, args.commit):
        notes = [GENERAL_NOTE]
    if not notes:
        print("release=false")
        return

    if args.version:
        version = args.version
    elif last:
        version = next_version(last)
    else:
        raise Error("There's no release to number this one after: give --version.")
    if not VERSION.fullmatch(version):
        raise Error(f"{version} isn't a version, such as 1.4.3.")
    if git("rev-parse", "-q", "--verify", f"refs/tags/{version}").returncode == 0:
        raise Error(f"There's a release {version} already.")

    Path(args.notes).write_text(as_markdown(notes))
    print("release=true")
    print(f"version={version}")


def notes(args: argparse.Namespace) -> None:
    # The commit is tagged already, as the build or the release it is, so
    # the one before it is the one before that.
    last = last_release(f"{args.commit}^", numbered=args.release)
    release_notes = notes_since(last, args.commit)
    if not args.release:
        # A development build: usually none, since most commits have none.
        sys.stdout.write(as_markdown(release_notes))
        return

    # A numbered release made by hand may have no notes of its own.
    sys.stdout.write(as_markdown(release_notes or [GENERAL_NOTE]))
    repository = os.environ.get("GITHUB_REPOSITORY", "clementine-player/Clementine")
    print(
        "\nDevelopment builds, from every change since: "
        f"[pre-releases](https://github.com/{repository}/releases)"
    )


def main() -> None:
    parser = argparse.ArgumentParser(
        description=__doc__.splitlines()[0],
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="\n".join(__doc__.splitlines()[1:]),
    )
    commands = parser.add_subparsers(required=True)

    plan_parser = commands.add_parser("plan", help="plan the next numbered release")
    plan_parser.add_argument("--commit", default="origin/master")
    plan_parser.add_argument("--version", help="instead of the next one, such as 1.5.0")
    plan_parser.add_argument(
        "--always",
        action="store_true",
        help="release any change since the last release, even without notes",
    )
    plan_parser.add_argument(
        "--notes", default="notes.md", help="where to write the notes"
    )
    plan_parser.set_defaults(command=plan)

    notes_parser = commands.add_parser("notes", help="a GitHub release's notes")
    notes_parser.add_argument("--commit", default="HEAD")
    notes_parser.add_argument(
        "--release", action="store_true", help="for a numbered release"
    )
    notes_parser.set_defaults(command=notes)

    args = parser.parse_args()
    try:
        args.command(args)
    except Error as error:
        # An annotation, run by GitHub Actions.
        print(f"::error::{error}", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
