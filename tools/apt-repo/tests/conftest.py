"""Test helpers: packages built on the fly, and a throwaway signing key."""

from __future__ import annotations

import io
import lzma
import shutil
import subprocess
import tarfile
from collections.abc import Callable
from pathlib import Path

import pytest
import zstandard

from clementine_apt.openpgp import Key
from clementine_apt.signers import LocalSigner

MakeDeb = Callable[..., Path]


def _tar(files: dict[str, bytes]) -> bytes:
    buffer = io.BytesIO()
    with tarfile.open(fileobj=buffer, mode="w") as tar:
        for name, data in files.items():
            info = tarfile.TarInfo(name)
            info.size = len(data)
            tar.addfile(info, io.BytesIO(data))
    return buffer.getvalue()


def _ar(members: list[tuple[str, bytes]]) -> bytes:
    out = b"!<arch>\n"
    for name, data in members:
        header = (
            f"{name:<16}{0:<12}{0:<6}{0:<6}{'100644':<8}{len(data):<10}".encode()
            + b"`\n"
        )
        out += header + data + (b"\n" if len(data) % 2 else b"")
    return out


@pytest.fixture
def make_deb(tmp_path: Path) -> MakeDeb:
    """Builds a small .deb, with its control.tar compressed as asked."""

    def make(
        version: str,
        arch: str = "amd64",
        package: str = "clementine",
        compression: str = "xz",
    ) -> Path:
        control = (
            f"Package: {package}\nVersion: {version}\nArchitecture: {arch}\n"
            "Maintainer: Clementine <nobody@example.com>\n"
            "Description: A music player\n Plays music.\n"
        ).encode()
        control_tar = _tar({"./control": control})
        if compression == "xz":
            compressed = lzma.compress(control_tar)
        elif compression == "zst":
            compressed = zstandard.ZstdCompressor().compress(control_tar)
        else:
            import gzip

            compressed = gzip.compress(control_tar)
        data = _ar(
            [
                ("debian-binary", b"2.0\n"),
                (f"control.tar.{compression}", compressed),
                ("data.tar.xz", lzma.compress(_tar({}))),
            ]
        )
        path = tmp_path / "debs" / f"{package}_{version.replace('~', '.')}_{arch}.deb"
        path.parent.mkdir(exist_ok=True)
        path.write_bytes(data)
        return path

    return make


@pytest.fixture(scope="session")
def key() -> Key:
    return Key(LocalSigner.generate(), 1_790_000_000, "Clementine test key")


@pytest.fixture
def keyring(key: Key, tmp_path: Path) -> Path:
    path = tmp_path / "keyring.gpg"
    path.write_bytes(key.public_key())
    return path


def verify(keyring: Path, *files: Path) -> None:
    """gpgv, and for detached signatures sqv where it's installed: apt uses
    one or the other."""
    subprocess.run(["gpgv", "--keyring", str(keyring), *map(str, files)], check=True)
    if len(files) == 2 and shutil.which("sqv"):
        subprocess.run(["sqv", "--keyring", str(keyring), *map(str, files)], check=True)
