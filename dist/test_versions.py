"""Checks that the versions cmake/Version.cmake gives builds sort in the order
the builds were made, by everything that compares them:

- GitHub's releases page, which lists a day's releases by their tags.
- dpkg and apt, for the PPA and our apt repository, and tools/apt-repo,
  which keeps the newest builds in that repository.
- rpm and dnf, for COPR, whose builds must also beat Fedora's own.
- Sparkle, which offers Mac users the feed's highest CFBundleVersion.

Each of these has put builds in the wrong order before. The versions come
from running Version.cmake itself, with FORCE_GIT_REVISION standing in for
git describe --long, so a change there that breaks an order fails here.
"""

from __future__ import annotations

import functools
import itertools
import re
import shutil
import subprocess
import tempfile
from dataclasses import dataclass
from pathlib import Path

import pytest

from clementine_apt.version import compare as apt_repo_compare

ROOT = Path(__file__).resolve().parent.parent

# What git describe --long says for a sequence of builds, oldest first: the
# numbered release on its tag, then development builds after it, with counts
# that change their number of digits, then the next releases. Weekly releases
# keep the counts far below 1000, past which GitHub would list 1000 below 999
# (test_counts_past_999_are_kept_whole).
BUILDS = [
    "1.4.4-0-g9cc1e0b08",
    "1.4.4-1-gae3cf3737",
    "1.4.4-9-g30c8967a3",
    "1.4.4-10-g74bac60ac",
    "1.4.4-14-gc4987e3d6",
    "1.4.4-99-g6df858722",
    "1.4.4-100-g82d7e953a",
    "1.4.4-282-gae9d5d8cd",
    "1.4.4-999-g50ce74be7",
    "1.4.5-0-gb06cd4bf6",
    "1.4.5-2-g1c362dd2a",
    "1.4.10-0-g833324d51",
    "1.4.10-3-ge1f0534c7",
    "1.5.0-0-gc2c559812",
    "1.5.0-1-g99535e161",
]

# The Fedora release a COPR build is built for, which rpm's %{?dist} adds to
# the release.
DIST = ".fc43"


@dataclass(frozen=True)
class Versions:
    describe: str
    display: str
    deb: str
    rpm_version: str
    rpm_release: str
    sparkle: str  # Also the tarball's version, so the GitHub release's tag.
    plist: str  # CFBundleVersion

    @property
    def tag(self) -> str:
        return self.describe.rsplit("-", 2)[0]

    @property
    def count(self) -> int:
        return int(self.describe.rsplit("-", 2)[1])


@functools.cache
def versions(describe: str) -> Versions:
    """What Version.cmake makes of describe --long's output."""
    cmake = shutil.which("cmake")
    assert cmake, "cmake is needed to run Version.cmake"
    names = ["DISPLAY", "DEB", "RPM_V", "RPM_R", "SPARKLE", "PLIST"]
    script = "".join(
        f'message(NOTICE "{name}=${{CLEMENTINE_VERSION_{name}}}")\n' for name in names
    )
    with tempfile.TemporaryDirectory() as temp:
        path = Path(temp) / "versions.cmake"
        path.write_text(f'include("{ROOT}/cmake/Version.cmake")\n{script}')
        result = subprocess.run(
            [
                cmake,
                f"-DCMAKE_SOURCE_DIR={ROOT}",
                f"-DFORCE_GIT_REVISION={describe}",
                "-P",
                str(path),
            ],
            capture_output=True,
            text=True,
            check=True,
        )
    values = dict(line.split("=", 1) for line in result.stderr.splitlines())
    return Versions(describe, *(values[name] for name in names))


def ordered_pairs() -> list[tuple[str, str]]:
    """Every pair of builds, older first."""
    return list(itertools.combinations(BUILDS, 2))


def pair_id(pair: tuple[str, str]) -> str:
    return f"{pair[0]}<{pair[1]}"


def is_numbered(v: Versions) -> bool:
    return v.count == 0


# GitHub ----------------------------------------------------------------------


def github_key(tag: str) -> tuple[tuple[int, int, int], int, str]:
    """Where GitHub lists a release among others made the same day.

    GitHub groups releases by their commit's date and orders each group by
    tag. It isn't documented how; this is what it has done: the major, minor
    and patch numbers compare as numbers, a release comes above its own
    pre-releases, and everything after the "-" compares as text. On
    2026-11-10 that put 1.4.5-9.dev-g30c8967a3 above 1.4.5-14.dev-gc4987e3d6
    (test_github_model_matches_what_github_did).
    """
    match = re.fullmatch(r"(\d+)\.(\d+)\.(\d+)(?:-(.+))?", tag)
    assert match, f"{tag} isn't a version GitHub would order"
    major, minor, patch, prerelease = match.groups()
    core = (int(major), int(minor), int(patch))
    if prerelease is None:
        return core, 1, ""
    return core, 0, prerelease


def test_github_model_matches_what_github_did() -> None:
    # The releases page on 2026-11-10, top to bottom: all built from commits
    # made that day, with tags from before the counts were padded. If the
    # model didn't reproduce this, it couldn't show that padding fixes it.
    listed = [
        "1.4.5-9.dev-g30c8967a3",
        "1.4.5-8.dev-g833324d51",
        "1.4.5-7.dev-g1c362dd2a",
        "1.4.5-14.dev-gc4987e3d6",
        "1.4.5-12.dev-ge1f0534c7",
        "1.4.5-11.dev-gc2c559812",
        "1.4.5-10.dev-g74bac60ac",
    ]
    assert sorted(listed, key=github_key, reverse=True) == listed
    # And on 2026-11-09, the numbered release above a later build of its own.
    listed = ["1.4.5", "1.4.5-2.dev-gb06cd4bf6"]
    assert sorted(listed, key=github_key, reverse=True) == listed


@pytest.mark.parametrize("pair", ordered_pairs(), ids=pair_id)
def test_github_lists_builds_in_order(pair: tuple[str, str]) -> None:
    older, newer = (versions(describe) for describe in pair)
    if is_numbered(older) and newer.tag == older.tag:
        pytest.xfail(
            "GitHub puts a release above its own pre-releases, so on the day "
            "of a numbered release, the development builds after it are "
            "listed below it."
        )
    assert github_key(older.sparkle) < github_key(newer.sparkle)


# dpkg ------------------------------------------------------------------------


def dpkg_older(older: str, newer: str) -> bool:
    dpkg = shutil.which("dpkg")
    assert dpkg, "dpkg is needed to compare Debian versions"
    return (
        subprocess.run([dpkg, "--compare-versions", older, "lt", newer]).returncode == 0
    )


@pytest.mark.parametrize("pair", ordered_pairs(), ids=pair_id)
def test_dpkg_orders_builds(pair: tuple[str, str]) -> None:
    # debian/changelog.in adds the Ubuntu or Debian release it's built for.
    older, newer = (f"{versions(describe).deb}~noble" for describe in pair)
    assert dpkg_older(older, newer)


@pytest.mark.parametrize("pair", ordered_pairs(), ids=pair_id)
def test_apt_repo_keeps_the_newest_builds(pair: tuple[str, str]) -> None:
    # It prunes all but the newest builds in each suite by its own comparison.
    older, newer = (f"{versions(describe).deb}~noble" for describe in pair)
    assert apt_repo_compare(older, newer) < 0


@pytest.mark.parametrize(
    ("older", "newer"),
    [
        # git describe's own form, before 1.4.3-13.
        ("1.4.3-12-g43226261f~noble", "1.4.3-13.dev-g4c062729d~noble"),
        # Before the counts were padded, after 1.4.5-14.
        ("1.4.5-14.dev-gc4987e3d6~noble", "1.4.5-015.dev-gc4987e3d6~noble"),
    ],
)
def test_dpkg_upgrades_across_version_schemes(older: str, newer: str) -> None:
    assert dpkg_older(older, newer)


# rpm -------------------------------------------------------------------------


def rpm_compare(a: str, b: str) -> int:
    """rpmvercmp: below, at or above 0 as a is older, the same or newer."""
    rpm = shutil.which("rpm")
    assert rpm, "rpm is needed to compare rpm versions (apt install rpm)"
    for value in (a, b):
        assert re.fullmatch(r"[\w.^~+]+", value), value
    result = subprocess.run(
        [rpm, "--eval", f'%{{lua:print(rpm.vercmp("{a}", "{b}"))}}'],
        capture_output=True,
        text=True,
        check=True,
    )
    return int(result.stdout)


def rpm_older(older: tuple[str, str], newer: tuple[str, str]) -> bool:
    """Whether dnf upgrades the (version, release) older to newer."""
    return (rpm_compare(older[0], newer[0]) or rpm_compare(older[1], newer[1])) < 0


def rpm_label(v: Versions) -> tuple[str, str]:
    return v.rpm_version, v.rpm_release + DIST


@pytest.mark.parametrize("pair", ordered_pairs(), ids=pair_id)
def test_rpm_orders_builds(pair: tuple[str, str]) -> None:
    older, newer = (rpm_label(versions(describe)) for describe in pair)
    assert rpm_older(older, newer)


@pytest.mark.parametrize("describe", BUILDS)
def test_rpm_version_stays_the_tag(describe: str) -> None:
    # rpm builds in a directory named after the version, which lands in the
    # debug info and so in ccache's hash: a version that changed with every
    # commit made every compile a cache miss.
    v = versions(describe)
    assert v.rpm_version == v.tag


@pytest.mark.parametrize(
    ("fedora", "copr"),
    [
        # Fedora's own package of an older release.
        ("1.4.2-3.fc43", "1.4.4-0-g9cc1e0b08"),
        ("1.4.2-3.fc43", "1.4.4-14-gc4987e3d6"),
        # Fedora's of the same release: COPR's builds with more commits since
        # the tag than Fedora's release number. Once 1.4.1-2.256.gdb9f503df,
        # which lost to Fedora's 1.4.1-10 on the 2 alone.
        ("1.4.1-10.fc43", "1.4.1-282-gae9d5d8cd"),
        ("1.4.1-10.fc43", "1.4.1-11-gae9d5d8cd"),
    ],
)
def test_copr_builds_beat_fedoras(fedora: str, copr: str) -> None:
    version, release = fedora.split("-")
    assert rpm_older((version, release), rpm_label(versions(copr)))


# Sparkle ---------------------------------------------------------------------


def sparkle_key(build: str) -> tuple[int, ...]:
    """How Sparkle orders CFBundleVersions that are only dotted numbers.

    Sparkle splits versions into runs of digits, periods and other
    characters, and compares runs of digits as numbers; when one version has
    more runs, a number makes it the newer. For dotted numbers that's how
    tuples of them compare.
    """
    return tuple(int(part) for part in build.split("."))


@pytest.mark.parametrize("describe", BUILDS)
def test_cfbundleversion_is_dotted_numbers(describe: str) -> None:
    # tools/mac-update won't publish anything else.
    assert re.fullmatch(r"\d+(\.\d+)*", versions(describe).plist)


@pytest.mark.parametrize("pair", ordered_pairs(), ids=pair_id)
def test_sparkle_orders_builds(pair: tuple[str, str]) -> None:
    older, newer = (versions(describe) for describe in pair)
    assert sparkle_key(older.plist) < sparkle_key(newer.plist)


# Telling numbered releases from development builds ----------------------------


@pytest.mark.parametrize("describe", BUILDS)
def test_development_builds_are_recognisable(describe: str) -> None:
    v = versions(describe)
    # all.yml leaves numbered releases to release.yml by this pattern.
    numbered = re.fullmatch(r"[0-9]+(\.[0-9]+)+", v.sparkle) is not None
    assert numbered == is_numbered(v)
    # release-monitoring.org's filter hides development builds by "-g", so
    # Fedora's bot only files bugs for numbered releases.
    assert ("-g" in v.sparkle) == (not is_numbered(v))
    # release_notes.py finds the previous build with describe --match "[0-9]*".
    assert v.sparkle[0].isdigit()
    # One version, wherever people see it.
    assert v.display == v.deb == v.sparkle


def test_counts_past_999_are_kept_whole() -> None:
    assert versions("1.4.4-1234-g50ce74be7").sparkle == "1.4.4-1234.dev-g50ce74be7"
