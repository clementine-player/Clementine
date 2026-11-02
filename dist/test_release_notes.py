"""Tests for release_notes.py, on throwaway git repositories."""

from __future__ import annotations

import argparse
import os
import re
import subprocess
from pathlib import Path

import pytest

import release_notes


class Repo:
    """A git repository whose commits are tagged as Clementine's are."""

    def __init__(self, path: Path) -> None:
        self.path = path
        self.day = 1
        self.git("init", "-q", "-b", "master")

    def git(self, *args: str) -> str:
        # Dated by self.day, which each commit moves on, and without the
        # user's own git configuration.
        date = f"2020-01-{self.day:02d}T12:00:00Z"
        env = {
            **os.environ,
            "GIT_CONFIG_GLOBAL": os.devnull,
            "GIT_CONFIG_SYSTEM": os.devnull,
            "GIT_AUTHOR_NAME": "Test",
            "GIT_AUTHOR_EMAIL": "test@example.com",
            "GIT_COMMITTER_NAME": "Test",
            "GIT_COMMITTER_EMAIL": "test@example.com",
            "GIT_AUTHOR_DATE": date,
            "GIT_COMMITTER_DATE": date,
        }
        return subprocess.run(
            ["git", *args],
            cwd=self.path,
            env=env,
            check=True,
            capture_output=True,
            text=True,
        ).stdout

    def commit(self, note: str | None = None) -> None:
        self.day += 1
        message = f"Change {self.day}"
        if note:
            message += f"\n\nRelease-note: {note}"
        self.git("commit", "-q", "--allow-empty", "-m", message)

    def release(self, version: str) -> None:
        """A numbered release: an annotated tag, as release.yml makes."""
        self.git("tag", "-a", version, "-m", f"Clementine {version}")

    def old_release(self, version: str) -> None:
        """A release from before release.yml, some of which are lightweight."""
        self.git("tag", version)

    def build(self, name: str) -> None:
        """A development build: a lightweight tag, as all.yml makes."""
        self.git("tag", name)


@pytest.fixture
def repo(tmp_path: Path, monkeypatch: pytest.MonkeyPatch) -> Repo:
    repo = Repo(tmp_path)
    monkeypatch.setattr(release_notes, "ROOT", tmp_path)
    return repo


def appstream(capsys: pytest.CaptureFixture[str]) -> str:
    release_notes.appstream(argparse.Namespace(commit="HEAD"))
    return capsys.readouterr().out


def versions(capsys: pytest.CaptureFixture[str]) -> list[str]:
    return re.findall(r'<release version="([^"]+)"', appstream(capsys))


def test_releases_on_another_branch_are_in_order(
    repo: Repo, capsys: pytest.CaptureFixture[str]
) -> None:
    # Like 1.3 and 1.3.1: lightweight tags on a branch master never merged.
    repo.commit()
    repo.old_release("1.2")
    repo.git("checkout", "-q", "-b", "1.3")
    repo.commit()
    repo.old_release("1.3")
    repo.commit()
    repo.old_release("1.3.1")
    repo.git("checkout", "-q", "master")
    repo.commit()
    repo.release("1.4.1")
    repo.commit("Something new.")
    repo.release("1.4.2")

    assert versions(capsys) == ["1.4.2", "1.4.1", "1.3.1", "1.3", "1.2"]


def test_only_numbered_releases_up_to_head(
    repo: Repo, capsys: pytest.CaptureFixture[str]
) -> None:
    repo.commit()
    repo.release("1.4.0rc1")
    repo.commit()
    repo.release("1.4.1")
    repo.commit()
    repo.build("1.4.1-1-gabcdef012")
    repo.commit("Something new.")
    repo.release("1.4.2")
    repo.commit()
    repo.build("1.4.2-1-gabcdef012")
    head = repo.git("rev-parse", "HEAD").strip()
    # A release made since, which the build from head doesn't have.
    repo.commit("Something newer.")
    repo.release("1.4.3")
    repo.git("checkout", "-q", head)

    assert versions(capsys) == ["1.4.2", "1.4.1"]


def test_notes_and_dates(repo: Repo, capsys: pytest.CaptureFixture[str]) -> None:
    repo.commit()
    repo.release("1.4.1")
    repo.commit("Fixed a crash.")
    repo.commit("Lyrics & covers <for> everyone.")
    repo.release("1.4.2")

    assert appstream(capsys) == (
        '\t\t<release version="1.4.2" date="2020-01-04">\n'
        "\t\t\t<description>\n"
        "\t\t\t\t<ul>\n"
        "\t\t\t\t\t<li>Fixed a crash.</li>\n"
        "\t\t\t\t\t<li>Lyrics &amp; covers &lt;for&gt; everyone.</li>\n"
        "\t\t\t\t</ul>\n"
        "\t\t\t</description>\n"
        "\t\t</release>\n"
        # Releases from before release notes have none.
        '\t\t<release version="1.4.1" date="2020-01-02">\n'
        "\t\t</release>\n"
    )


def test_no_releases(repo: Repo, capsys: pytest.CaptureFixture[str]) -> None:
    repo.commit()
    repo.build("1.4.1-1-gabcdef012")

    assert appstream(capsys) == ""
