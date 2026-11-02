#!/usr/bin/env python3
"""Tests for release_notes.py, on throwaway git repositories.

Run from the repository's root:

  python3 -m unittest discover -s dist -p 'test_*.py'
"""

import argparse
import contextlib
import importlib.util
import io
import os
import re
import subprocess
import tempfile
import unittest
from pathlib import Path

spec = importlib.util.spec_from_file_location(
    "release_notes", Path(__file__).resolve().parent / "release_notes.py"
)
assert spec and spec.loader
release_notes = importlib.util.module_from_spec(spec)
spec.loader.exec_module(release_notes)


class Repo:
    """A git repository whose commits are tagged as Clementine's are."""

    def __init__(self, path: Path) -> None:
        self.path = path
        self.day = 1
        self.git("init", "-q", "-b", "master")

    def git(self, *args: str) -> str:
        # Dated by self.day, which each commit moves on, and without the
        # user's own git configuration.
        env = {
            **os.environ,
            "GIT_CONFIG_GLOBAL": os.devnull,
            "GIT_CONFIG_SYSTEM": os.devnull,
            "GIT_AUTHOR_NAME": "Test",
            "GIT_AUTHOR_EMAIL": "test@example.com",
            "GIT_COMMITTER_NAME": "Test",
            "GIT_COMMITTER_EMAIL": "test@example.com",
            "GIT_AUTHOR_DATE": f"2020-01-{self.day:02d}T12:00:00Z",
            "GIT_COMMITTER_DATE": f"2020-01-{self.day:02d}T12:00:00Z",
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


class AppstreamTest(unittest.TestCase):
    def setUp(self) -> None:
        directory = tempfile.TemporaryDirectory()
        self.addCleanup(directory.cleanup)
        self.repo = Repo(Path(directory.name))
        root = release_notes.ROOT
        release_notes.ROOT = self.repo.path
        self.addCleanup(setattr, release_notes, "ROOT", root)

    def appstream(self) -> str:
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            release_notes.appstream(argparse.Namespace(commit="HEAD"))
        return output.getvalue()

    def versions(self) -> list[str]:
        return re.findall(r'<release version="([^"]+)"', self.appstream())

    def test_releases_on_another_branch_are_in_order(self) -> None:
        # Like 1.3 and 1.3.1: lightweight tags on a branch master never merged.
        self.repo.commit()
        self.repo.old_release("1.2")
        self.repo.git("checkout", "-q", "-b", "1.3")
        self.repo.commit()
        self.repo.old_release("1.3")
        self.repo.commit()
        self.repo.old_release("1.3.1")
        self.repo.git("checkout", "-q", "master")
        self.repo.commit()
        self.repo.release("1.4.1")
        self.repo.commit("Something new.")
        self.repo.release("1.4.2")

        self.assertEqual(self.versions(), ["1.4.2", "1.4.1", "1.3.1", "1.3", "1.2"])

    def test_only_numbered_releases_up_to_head(self) -> None:
        self.repo.commit()
        self.repo.release("1.4.0rc1")
        self.repo.commit()
        self.repo.release("1.4.1")
        self.repo.commit()
        self.repo.build("1.4.1-1-gabcdef012")
        self.repo.commit("Something new.")
        self.repo.release("1.4.2")
        self.repo.commit()
        self.repo.build("1.4.2-1-gabcdef012")
        head = self.repo.git("rev-parse", "HEAD").strip()
        # A release made since, which the build from head doesn't have.
        self.repo.commit("Something newer.")
        self.repo.release("1.4.3")
        self.repo.git("checkout", "-q", head)

        self.assertEqual(self.versions(), ["1.4.2", "1.4.1"])

    def test_notes_and_dates(self) -> None:
        self.repo.commit()
        self.repo.release("1.4.1")
        self.repo.commit("Fixed a crash.")
        self.repo.commit("Lyrics & covers <for> everyone.")
        self.repo.release("1.4.2")

        self.assertEqual(
            self.appstream(),
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
            "\t\t</release>\n",
        )

    def test_no_releases(self) -> None:
        self.repo.commit()
        self.repo.build("1.4.1-1-gabcdef012")

        self.assertEqual(self.appstream(), "")


if __name__ == "__main__":
    unittest.main()
