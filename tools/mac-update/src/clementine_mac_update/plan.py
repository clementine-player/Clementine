"""Decides whether there's a macOS update to put in the Sparkle feed, and which.

Every push to master makes a GitHub release with a DMG, but the feed gets one
a week at most, and only when there's something to tell users: the commits
since the last feed update carry release notes, "Release-note:" trailers in
their messages, one line each, written for users. Commits without one
(refactoring, tests, CI) don't make an update, unless `always` is set (a run
by hand): then any commit makes one, with a general note if none has its own.

The update is the newest release on master with a DMG. Each one published is
marked with a tag, sparkle/<release>, at the same commit. The notes are those
of the commits after the last such tag, up to the release's: commits after it
aren't in its DMG.
"""

from __future__ import annotations

import json
import subprocess
import sys
from collections.abc import Callable
from dataclasses import dataclass
from pathlib import Path

TAG_PREFIX = "sparkle/"
GENERAL_NOTE = "Fixes and improvements."
# How many of the newest releases to look through for one with a DMG.
RELEASES_TO_SEARCH = 10
# Each commit's Release-note: trailers, one per line.
NOTES_FORMAT = "%(trailers:key=Release-note,valueonly,separator=%x0A)"

# A release's tag -> its DMG's file name, or None when it has none.
FindDmg = Callable[[str], str | None]


@dataclass(frozen=True)
class Update:
    release: str
    dmg: str
    last: str | None
    notes: list[str]


def git(repo: Path, *args: str) -> str:
    return subprocess.run(
        ["git", *args], cwd=repo, check=True, capture_output=True, text=True
    ).stdout


def github_dmg(repository: str) -> FindDmg:
    """Finds a release's DMG with the GitHub CLI."""

    def find(tag: str) -> str | None:
        result = subprocess.run(
            ["gh", "release", "view", tag, "--repo", repository, "--json", "assets"],
            capture_output=True,
            text=True,
        )
        if result.returncode != 0:
            return None
        names: list[str] = [a["name"] for a in json.loads(result.stdout)["assets"]]
        return next((n for n in names if n.endswith(".dmg")), None)

    return find


def plan(
    repo: Path, master: str, find_dmg: FindDmg, always: bool = False
) -> Update | None:
    """The update to publish, or None when there's none (saying why on stderr)."""
    tags = git(
        repo, "tag", "--list", f"{TAG_PREFIX}*", "--merged", master, "--sort=-v:refname"
    ).split()
    last = tags[0] if tags else None

    releases = git(
        repo, "tag", "--list", "[0-9]*", "--merged", master, "--sort=-v:refname"
    ).split()
    for release in releases[:RELEASES_TO_SEARCH]:
        dmg = find_dmg(release)
        if dmg:
            break
    else:
        _say(f"No release on {master} has a DMG.")
        return None

    if last and _is_ancestor(repo, release, last):
        _say(f"{release} is already in the feed ({last}).")
        return None

    commits = f"{last}..{release}" if last else release
    log = git(repo, "log", "--reverse", f"--format={NOTES_FORMAT}", commits)
    notes = [line.strip() for line in log.splitlines() if line.strip()]
    if not notes and always:
        notes = [GENERAL_NOTE]
    if not notes:
        _say(f"No release notes in {commits}.")
        return None
    return Update(release=release, dmg=dmg, last=last, notes=notes)


def _is_ancestor(repo: Path, commit: str, of: str) -> bool:
    return (
        subprocess.run(
            ["git", "merge-base", "--is-ancestor", commit, of], cwd=repo
        ).returncode
        == 0
    )


def _say(message: str) -> None:
    print(message, file=sys.stderr)
