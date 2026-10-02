from __future__ import annotations

import hashlib
import io
import tarfile
from pathlib import Path

import pytest

from clementine_mac_update import sparkle


def _archive() -> bytes:
    """A small Sparkle-*.tar.xz, laid out like the real one."""
    buffer = io.BytesIO()
    with tarfile.open(fileobj=buffer, mode="w:xz") as tar:

        def add(name: str, data: bytes = b"", mode: int = 0o644) -> None:
            info = tarfile.TarInfo(name)
            info.size = len(data)
            info.mode = mode
            tar.addfile(info, io.BytesIO(data))

        def link(name: str, target: str) -> None:
            info = tarfile.TarInfo(name)
            info.type = tarfile.SYMTYPE
            info.linkname = target
            tar.addfile(info)

        add("./Sparkle.framework/Versions/B/Sparkle", b"framework", 0o755)
        link("./Sparkle.framework/Versions/Current", "B")
        link("./Sparkle.framework/Sparkle", "Versions/Current/Sparkle")
        add("./bin/sign_update", b"tool", 0o755)
        add("./Sparkle Test App.app/Contents/Info.plist", b"test app")
        add("./Symbols/Sparkle.dSYM", b"symbols")
    return buffer.getvalue()


def test_unpacks_the_framework_and_tools(tmp_path: Path) -> None:
    data = _archive()
    sparkle.extract(io.BytesIO(data), tmp_path, hashlib.sha256(data).hexdigest())

    assert (tmp_path / "Sparkle.framework" / "Sparkle").read_bytes() == b"framework"
    assert (tmp_path / "Sparkle.framework" / "Sparkle").is_symlink()
    assert (tmp_path / "bin" / "sign_update").stat().st_mode & 0o111
    assert sorted(p.name for p in tmp_path.iterdir()) == ["Sparkle.framework", "bin"]


def test_refuses_an_archive_with_another_checksum(tmp_path: Path) -> None:
    with pytest.raises(SystemExit, match="sha256"):
        sparkle.extract(io.BytesIO(_archive()), tmp_path / "out", "0" * 64)
    assert not (tmp_path / "out").exists()
