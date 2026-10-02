"""The pinned Sparkle release: its framework for the build, its tools to publish.

To move to a newer Sparkle, change VERSION and SHA256, from the release's page
on https://github.com/sparkle-project/Sparkle/releases (each asset lists its
sha256).
"""

from __future__ import annotations

import hashlib
import io
import shutil
import tarfile
import tempfile
import time
import urllib.request
from pathlib import Path

VERSION = "2.10.0"
SHA256 = "c2bf58aa8387266ac179357b1415d6f2635f044da8be41042af32425dae6da0c"
URL = (
    "https://github.com/sparkle-project/Sparkle/releases/download/"
    f"{VERSION}/Sparkle-{VERSION}.tar.xz"
)

# The parts used: the framework for the build (cmake -DCMAKE_FRAMEWORK_PATH=
# the directory it's in), and bin/sign_update to sign updates. Not the test
# app or the debug symbols.
PARTS = ("Sparkle.framework", "bin")


def fetch(dest: Path, url: str = URL, sha256: str = SHA256) -> None:
    """Downloads the release, checks it, and unpacks its parts into dest."""
    with tempfile.TemporaryFile() as archive:
        for attempt in range(3):
            try:
                with urllib.request.urlopen(url, timeout=60) as response:
                    shutil.copyfileobj(response, archive)
                break
            except OSError:
                if attempt == 2:
                    raise
                archive.seek(0)
                archive.truncate()
                time.sleep(5)
        archive.seek(0)
        extract(archive, dest, sha256)


def extract(archive: io.BufferedIOBase, dest: Path, sha256: str) -> None:
    """Checks the archive's sha256, then unpacks its parts into dest."""
    digest = hashlib.file_digest(archive, "sha256").hexdigest()
    if digest != sha256:
        raise SystemExit(f"Sparkle's archive has the sha256 {digest}, not {sha256}")
    archive.seek(0)
    dest.mkdir(parents=True, exist_ok=True)
    with tarfile.open(fileobj=archive, mode="r:xz") as tar:
        members = [m for m in tar.getmembers() if _part(m.name) in PARTS]
        # "data" keeps the framework's own links between its versions, and
        # refuses anything that would land outside dest.
        tar.extractall(dest, members=members, filter="data")


def _part(name: str) -> str:
    return name.removeprefix("./").split("/", 1)[0]
