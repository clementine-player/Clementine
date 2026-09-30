"""An apt repository in a directory, laid out as apt expects to find it:

    pool/main/c/clementine/<package>_<version>_<arch>.deb
    dists/<suite>/main/binary-<arch>/Packages{,.gz}
    dists/<suite>/main/binary-<arch>/by-hash/SHA256/<hash>
    dists/<suite>/{Release,Release.gpg,InRelease}

Only the index files have to be in the directory to start with: the
packages already published stay where they are, and the Packages files say
what they are. Each suite is a distribution release (noble, trixie...),
taken from the end of the package's version (1.4.1-251-ga588bb3f8~noble).
"""

from __future__ import annotations

import datetime
import gzip
import hashlib
import re
import shutil
from dataclasses import dataclass, field
from pathlib import Path

from clementine_apt import deb, version
from clementine_apt.openpgp import Key

COMPONENT = "main"
_SUITE_SUFFIX = re.compile(r"~([a-z]+)$")


@dataclass
class Suite:
    name: str
    # Architecture → the Packages file's stanzas.
    packages: dict[str, list[deb.Stanza]] = field(default_factory=dict)


def _digest(data: bytes, algorithm: str) -> str:
    return hashlib.new(algorithm, data).hexdigest()


class Repository:
    def __init__(
        self, root: Path, origin: str = "Clementine", label: str = "Clementine"
    ) -> None:
        self.root = root
        self.origin = origin
        self.label = label
        self.suites: dict[str, Suite] = {}
        self._load()

    def _load(self) -> None:
        dists = self.root / "dists"
        if not dists.is_dir():
            return
        for packages in sorted(dists.glob(f"*/{COMPONENT}/binary-*/Packages")):
            suite = self._suite(packages.parents[2].name)
            arch = packages.parent.name.removeprefix("binary-")
            suite.packages[arch] = deb.parse(packages.read_text())

    def _suite(self, name: str) -> Suite:
        return self.suites.setdefault(name, Suite(name))

    def add(self, path: Path) -> str:
        """Adds the package at |path|, replacing one of the same version.
        Returns the suite it went in."""
        control = deb.read_control(path)
        package = deb.get(control, "Package")
        ver = deb.get(control, "Version")
        arch = deb.get(control, "Architecture")
        if not package or not ver or not arch:
            raise ValueError(f"{path}: no Package, Version or Architecture")
        match = _SUITE_SUFFIX.search(ver)
        if not match:
            raise ValueError(f"{path}: version {ver} doesn't end with ~<suite>")
        suite = self._suite(match.group(1))

        source = (deb.get(control, "Source") or package).split(" ")[0]
        prefix = source[:4] if source.startswith("lib") else source[0]
        # The epoch, if any, isn't part of the file name.
        filename = (
            f"pool/{COMPONENT}/{prefix}/{source}/"
            f"{package}_{ver.split(':')[-1]}_{arch}.deb"
        )
        target = self.root / filename
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(path, target)

        data = target.read_bytes()
        stanza: deb.Stanza = [
            *control,
            ("Filename", filename),
            ("Size", str(len(data))),
            ("MD5sum", _digest(data, "md5")),
            ("SHA1", _digest(data, "sha1")),
            ("SHA256", _digest(data, "sha256")),
        ]
        stanzas = suite.packages.setdefault(arch, [])
        stanzas[:] = [
            s
            for s in stanzas
            if (deb.get(s, "Package"), deb.get(s, "Version")) != (package, ver)
        ]
        stanzas.append(stanza)
        return suite.name

    def prune(self, keep: int) -> list[str]:
        """Keeps the |keep| newest versions of each package in each suite and
        architecture. Returns the pool files nothing refers to any more."""
        before = self._filenames()
        for suite in self.suites.values():
            for arch, stanzas in suite.packages.items():
                by_package: dict[str, list[deb.Stanza]] = {}
                for stanza in stanzas:
                    by_package.setdefault(deb.get(stanza, "Package") or "", []).append(
                        stanza
                    )
                kept: list[deb.Stanza] = []
                for versions in by_package.values():
                    versions.sort(
                        key=lambda s: version.sort_key(deb.get(s, "Version") or ""),
                        reverse=True,
                    )
                    kept.extend(versions[:keep])
                suite.packages[arch] = kept
        gone = sorted(before - self._filenames())
        for filename in gone:
            (self.root / filename).unlink(missing_ok=True)
        return gone

    def _filenames(self) -> set[str]:
        return {
            deb.get(stanza, "Filename") or ""
            for suite in self.suites.values()
            for stanzas in suite.packages.values()
            for stanza in stanzas
        }

    def write(self, key: Key, now: datetime.datetime) -> None:
        """Writes every suite's index files, signed by |key|."""
        for suite in self.suites.values():
            self._write_suite(suite, key, now)

    def _write_suite(self, suite: Suite, key: Key, now: datetime.datetime) -> None:
        base = self.root / "dists" / suite.name
        # Relative to base → contents, for the Release file to list.
        indexes: dict[str, bytes] = {}
        for arch in sorted(suite.packages):
            stanzas = sorted(
                suite.packages[arch],
                key=lambda s: (
                    deb.get(s, "Package") or "",
                    version.sort_key(deb.get(s, "Version") or ""),
                ),
            )
            text = "\n".join(deb.format_stanza(s) for s in stanzas).encode()
            directory = f"{COMPONENT}/binary-{arch}"
            indexes[f"{directory}/Packages"] = text
            # mtime=0 so the same packages give the same file.
            indexes[f"{directory}/Packages.gz"] = gzip.compress(text, mtime=0)

        for name, data in indexes.items():
            path = base / name
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(data)
            # A copy under its hash, which clients fetch when the Release file
            # says to: then one that fetched the Release file just before an
            # update still finds the Packages file it lists.
            by_hash = path.parent / "by-hash" / "SHA256" / _digest(data, "sha256")
            by_hash.parent.mkdir(parents=True, exist_ok=True)
            by_hash.write_bytes(data)

        lines = [
            f"Origin: {self.origin}",
            f"Label: {self.label}",
            f"Suite: {suite.name}",
            f"Codename: {suite.name}",
            f"Date: {now.astimezone(datetime.UTC):%a, %d %b %Y %H:%M:%S} UTC",
            f"Architectures: {' '.join(sorted(suite.packages))}",
            f"Components: {COMPONENT}",
            f"Description: Clementine for {suite.name}",
            "Acquire-By-Hash: yes",
        ]
        width = max((len(str(len(data))) for data in indexes.values()), default=1)
        for title, algorithm in (
            ("MD5Sum", "md5"),
            ("SHA1", "sha1"),
            ("SHA256", "sha256"),
        ):
            lines.append(f"{title}:")
            for name, data in indexes.items():
                size = str(len(data)).rjust(width)
                lines.append(f" {_digest(data, algorithm)} {size} {name}")
        release = "\n".join(lines) + "\n"

        timestamp = int(now.timestamp())
        (base / "Release").write_text(release)
        (base / "Release.gpg").write_text(
            key.sign_detached(release.encode(), timestamp)
        )
        (base / "InRelease").write_text(key.sign_cleartext(release, timestamp))
