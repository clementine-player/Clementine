from __future__ import annotations

import datetime
import gzip
import hashlib
from pathlib import Path

import pytest
from conftest import MakeDeb, verify

from clementine_apt import deb
from clementine_apt.openpgp import Key
from clementine_apt.repo import Repository

NOW = datetime.datetime(2026, 9, 30, 12, 0, tzinfo=datetime.UTC)


def packages(root: Path, suite: str, arch: str) -> list[deb.Stanza]:
    path = root / "dists" / suite / "main" / f"binary-{arch}" / "Packages"
    return deb.parse(path.read_text())


@pytest.mark.parametrize("compression", ["xz", "zst", "gz"])
def test_reads_control(make_deb: MakeDeb, compression: str) -> None:
    control = deb.read_control(make_deb("1.0~noble", compression=compression))
    assert deb.get(control, "Package") == "clementine"
    assert deb.get(control, "Version") == "1.0~noble"
    assert deb.get(control, "Description") == "A music player\n Plays music."


def test_publishes_signed_suites(
    make_deb: MakeDeb, key: Key, keyring: Path, tmp_path: Path
) -> None:
    root = tmp_path / "repo"
    repo = Repository(root)
    assert repo.add(make_deb("1.4.1-1-gaaaaaaa~noble")) == "noble"
    repo.add(make_deb("1.4.1-1-gaaaaaaa~noble", arch="arm64"))
    repo.add(make_deb("1.4.1-1-gaaaaaaa~bookworm"))
    repo.write(key, NOW)

    assert sorted(p.name for p in (root / "dists").iterdir()) == ["bookworm", "noble"]
    [stanza] = packages(root, "noble", "amd64")
    filename = deb.get(stanza, "Filename")
    assert (
        filename == "pool/main/c/clementine/clementine_1.4.1-1-gaaaaaaa~noble_amd64.deb"
    )
    pool = (root / filename).read_bytes()
    assert deb.get(stanza, "SHA256") == hashlib.sha256(pool).hexdigest()
    assert deb.get(stanza, "Size") == str(len(pool))

    noble = root / "dists" / "noble"
    verify(keyring, noble / "Release.gpg", noble / "Release")
    verify(keyring, noble / "InRelease")
    release = (noble / "Release").read_text()
    assert "Architectures: amd64 arm64\n" in release
    assert "Date: Wed, 30 Sep 2026 12:00:00 UTC\n" in release
    # Every index it lists is there, with that hash, and under its hash too.
    sha256 = release.split("SHA256:\n")[1].splitlines()
    assert len(sha256) == 4
    for line in sha256:
        digest, _size, name = line.split()
        index = noble / name
        assert hashlib.sha256(index.read_bytes()).hexdigest() == digest
        assert (index.parent / "by-hash" / "SHA256" / digest).exists()
    gz = noble / "main" / "binary-amd64" / "Packages.gz"
    assert (
        gzip.decompress(gz.read_bytes())
        == (noble / "main/binary-amd64/Packages").read_bytes()
    )


def test_picks_up_where_it_left_off(
    make_deb: MakeDeb, key: Key, tmp_path: Path
) -> None:
    root = tmp_path / "repo"
    first = Repository(root)
    first.add(make_deb("1.4.1-1-gaaaaaaa~noble"))
    first.write(key, NOW)

    # A later run has only the index files, not the packages.
    pool = root / "pool"
    for path in list(pool.rglob("*.deb")):
        path.unlink()
    second = Repository(root)
    second.add(make_deb("1.4.1-2-gbbbbbbb~noble"))
    second.write(key, NOW)
    versions = [deb.get(s, "Version") for s in packages(root, "noble", "amd64")]
    assert versions == ["1.4.1-1-gaaaaaaa~noble", "1.4.1-2-gbbbbbbb~noble"]


def test_same_version_again_replaces_it(
    make_deb: MakeDeb, key: Key, tmp_path: Path
) -> None:
    repo = Repository(tmp_path / "repo")
    repo.add(make_deb("1.0~noble"))
    repo.add(make_deb("1.0~noble"))
    repo.write(key, NOW)
    assert len(packages(tmp_path / "repo", "noble", "amd64")) == 1


def test_prunes_oldest_versions(make_deb: MakeDeb, key: Key, tmp_path: Path) -> None:
    root = tmp_path / "repo"
    repo = Repository(root)
    for n in (9, 10, 11):
        repo.add(make_deb(f"1.4.1-{n}-gaaaaaaa~noble"))
    repo.add(make_deb("1.4.1-9-gaaaaaaa~jammy"))
    gone = repo.prune(keep=2)
    repo.write(key, NOW)

    assert gone == [
        "pool/main/c/clementine/clementine_1.4.1-9-gaaaaaaa~noble_amd64.deb"
    ]
    assert not (root / gone[0]).exists()
    versions = [deb.get(s, "Version") for s in packages(root, "noble", "amd64")]
    # Sorted by version, not as text: 9 before 10.
    assert versions == ["1.4.1-10-gaaaaaaa~noble", "1.4.1-11-gaaaaaaa~noble"]
    assert len(packages(root, "jammy", "amd64")) == 1


def test_needs_a_suite(make_deb: MakeDeb, tmp_path: Path) -> None:
    with pytest.raises(ValueError, match="~<suite>"):
        Repository(tmp_path / "repo").add(make_deb("1.0"))
