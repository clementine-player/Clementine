"""Decides whether there's a macOS update to put in the Sparkle feed, and which.

Every push to master makes a development build, a GitHub pre-release named
after `git describe` (1.4.2-55-g4c062729d), and every week with release notes
makes a numbered release (1.4.3; see dist/release_notes.py). The feed gets the
numbered releases only: the newest one with a DMG.

Each one published is marked with a tag, sparkle/<release>, at the same
commit. The notes are those of the commits after the last such tag, up to the
release's: "Release-note:" trailers in their messages, one line each, written
for users. A release always has some, but when the commits since the last
update have none (a release made by hand), the update gets a general note.
"""

from __future__ import annotations

import json
import re
import subprocess
import sys
from collections.abc import Callable
from dataclasses import dataclass
from pathlib import Path

TAG_PREFIX = "sparkle/"
GENERAL_NOTE = "Fixes and improvements."
# A numbered release's tag, as opposed to a development build's.
NUMBERED = re.compile(r"[0-9]+(\.[0-9]+)+")
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


def plan(repo: Path, master: str, find_dmg: FindDmg) -> Update | None:
    """The update to publish, or None when there's none (saying why on stderr)."""
    tags = git(
        repo, "tag", "--list", f"{TAG_PREFIX}*", "--merged", master, "--sort=-v:refname"
    ).split()
    last = tags[0] if tags else None

    releases = [
        tag
        for tag in git(
            repo, "tag", "--list", "[0-9]*", "--merged", master, "--sort=-v:refname"
        ).split()
        if NUMBERED.fullmatch(tag)
    ]
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
    if not notes:
        notes = [GENERAL_NOTE]
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
