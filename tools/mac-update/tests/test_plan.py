from __future__ import annotations

import subprocess
from pathlib import Path

import pytest

from clementine_mac_update import plan

# Without the user's own git configuration: an identity, and no signing.
GIT = [
    "git",
    *("-c", "user.name=Test", "-c", "user.email=test@example.com"),
    *("-c", "commit.gpgsign=false", "-c", "tag.gpgsign=false"),
]


class Repo:
    """A git repository whose commits become releases, as master's do."""

    def __init__(self, path: Path) -> None:
        self.path = path
        self.dmgs: dict[str, str] = {}
        self.count = 0
        self._git("init", "-q", "-b", "master")

    def _git(self, *args: str) -> None:
        subprocess.run(
            [*GIT, *args],
            cwd=self.path,
            check=True,
            capture_output=True,
        )

    def commit(self, subject: str, note: str | None = None, dmg: bool = True) -> str:
        """Commits, and releases it as 1.4.1-<n>, with a DMG unless dmg=False."""
        message = subject + (f"\n\nRelease-note: {note}" if note else "")
        self._git("commit", "-q", "--allow-empty", "-m", message)
        self.count += 1
        release = f"1.4.1-{self.count}-gabc{self.count}"
        self._git("tag", release)
        if dmg:
            self.dmgs[release] = f"clementine-{release}.dmg"
        return release

    def mark_published(self, release: str) -> None:
        self._git("tag", f"sparkle/{release}", release)

    def next_update(self, always: bool = False) -> plan.Update | None:
        return plan.plan(self.path, "master", self.dmgs.get, always=always)


@pytest.fixture
def repo(tmp_path: Path) -> Repo:
    return Repo(tmp_path)


def test_publishes_the_newest_release_with_the_notes_since_the_last(
    repo: Repo,
) -> None:
    repo.commit("Old", note="Already published.")
    last = repo.commit("Published")
    repo.mark_published(last)
    repo.commit("Lyrics", note="Shows lyrics.")
    repo.commit("Refactor")
    newest = repo.commit("Fix", note="Fixes a crash.")

    assert repo.next_update() == plan.Update(
        release=newest,
        dmg=f"clementine-{newest}.dmg",
        last=f"sparkle/{last}",
        notes=["Shows lyrics.", "Fixes a crash."],
    )


def test_nothing_to_publish_without_notes(repo: Repo) -> None:
    repo.mark_published(repo.commit("Published", note="Old news."))
    repo.commit("Refactor")
    assert repo.next_update() is None


def test_run_by_hand_publishes_without_notes(repo: Repo) -> None:
    repo.mark_published(repo.commit("Published", note="Old news."))
    newest = repo.commit("Refactor")
    update = repo.next_update(always=True)
    assert update is not None
    assert update.release == newest
    assert update.notes == [plan.GENERAL_NOTE]


def test_nothing_to_publish_when_the_newest_is_published(repo: Repo) -> None:
    repo.mark_published(repo.commit("Published", note="Old news."))
    assert repo.next_update(always=True) is None


def test_skips_releases_without_a_dmg(repo: Repo) -> None:
    with_dmg = repo.commit("Lyrics", note="Shows lyrics.")
    repo.commit("Fix", note="Fixes a crash.", dmg=False)

    update = repo.next_update()
    assert update is not None
    assert update.release == with_dmg
    # The fix isn't in that DMG, so it waits for the next update.
    assert update.notes == ["Shows lyrics."]


def test_takes_every_note_of_a_commit(repo: Repo) -> None:
    repo.commit("Lyrics", note="Shows lyrics.\nRelease-note: Scrolls them too.")
    update = repo.next_update()
    assert update is not None
    assert update.notes == ["Shows lyrics.", "Scrolls them too."]
