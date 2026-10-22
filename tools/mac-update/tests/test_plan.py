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
    """A git repository whose commits become development builds, as master's
    do, and some of them numbered releases."""

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

    def commit(self, subject: str, note: str | None = None) -> str:
        """Commits, and makes it a development build, 1.4.2-<n>, with a DMG."""
        message = subject + (f"\n\nRelease-note: {note}" if note else "")
        self._git("commit", "-q", "--allow-empty", "-m", message)
        self.count += 1
        build = f"1.4.2-{self.count}-gabc{self.count}"
        self._git("tag", build)
        self.dmgs[build] = f"clementine-{build}.dmg"
        return build

    def release(self, version: str, dmg: bool = True) -> str:
        """Releases the last commit as version, with a DMG unless dmg=False."""
        self._git("tag", "-a", version, "-m", f"Clementine {version}")
        if dmg:
            self.dmgs[version] = f"clementine-{version}.dmg"
        return version

    def mark_published(self, release: str) -> None:
        self._git("tag", f"sparkle/{release}", release)

    def next_update(self) -> plan.Update | None:
        return plan.plan(self.path, "master", self.dmgs.get)


@pytest.fixture
def repo(tmp_path: Path) -> Repo:
    return Repo(tmp_path)


def test_publishes_the_newest_release_with_the_notes_since_the_last(
    repo: Repo,
) -> None:
    repo.commit("Old", note="Already published.")
    repo.commit("Published")
    last = repo.release("1.4.3")
    repo.mark_published(last)
    repo.commit("Lyrics", note="Shows lyrics.")
    repo.commit("Refactor")
    repo.commit("Fix", note="Fixes a crash.")
    newest = repo.release("1.4.4")

    assert repo.next_update() == plan.Update(
        release=newest,
        dmg="clementine-1.4.4.dmg",
        last="sparkle/1.4.3",
        notes=["Shows lyrics.", "Fixes a crash."],
    )


def test_skips_development_builds(repo: Repo) -> None:
    repo.commit("Published", note="Old news.")
    repo.mark_published(repo.release("1.4.3"))
    repo.commit("Lyrics", note="Shows lyrics.")
    assert repo.next_update() is None


def test_a_release_without_notes_gets_the_general_note(repo: Repo) -> None:
    repo.commit("Published", note="Old news.")
    repo.mark_published(repo.release("1.4.3"))
    repo.commit("Refactor")
    newest = repo.release("1.4.4")
    update = repo.next_update()
    assert update is not None
    assert update.release == newest
    assert update.notes == [plan.GENERAL_NOTE]


def test_nothing_to_publish_when_the_newest_is_published(repo: Repo) -> None:
    repo.commit("Published", note="Old news.")
    repo.mark_published(repo.release("1.4.3"))
    assert repo.next_update() is None


def test_skips_releases_without_a_dmg(repo: Repo) -> None:
    repo.commit("Lyrics", note="Shows lyrics.")
    with_dmg = repo.release("1.4.3")
    repo.commit("Fix", note="Fixes a crash.")
    repo.release("1.4.4", dmg=False)

    update = repo.next_update()
    assert update is not None
    assert update.release == with_dmg
    # The fix isn't in that DMG, so it waits for the next update.
    assert update.notes == ["Shows lyrics."]


def test_orders_releases_by_version(repo: Repo) -> None:
    repo.commit("Lyrics", note="Shows lyrics.")
    repo.release("1.4.9")
    repo.commit("Fix", note="Fixes a crash.")
    newest = repo.release("1.4.10")
    update = repo.next_update()
    assert update is not None
    assert update.release == newest


def test_takes_every_note_of_a_commit(repo: Repo) -> None:
    repo.commit("Lyrics", note="Shows lyrics.\nRelease-note: Scrolls them too.")
    repo.release("1.4.3")
    update = repo.next_update()
    assert update is not None
    assert update.notes == ["Shows lyrics.", "Scrolls them too."]
