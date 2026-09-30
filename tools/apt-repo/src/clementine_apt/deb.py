"""Reads .deb packages, and the control-file format Packages files share."""

from __future__ import annotations

import gzip
import io
import lzma
import tarfile
from collections.abc import Iterator
from pathlib import Path

import zstandard

# A stanza keeps its fields in order, since Packages files are read by people
# too. Values of multi-line fields keep their continuation lines.
Stanza = list[tuple[str, str]]


def get(stanza: Stanza, name: str) -> str | None:
    lowered = name.lower()
    for field, value in stanza:
        if field.lower() == lowered:
            return value
    return None


def parse(text: str) -> list[Stanza]:
    """The stanzas of a control file or Packages file."""
    stanzas: list[Stanza] = []
    current: Stanza = []
    for line in text.splitlines():
        if not line.strip():
            if current:
                stanzas.append(current)
                current = []
        elif line[0] in " \t":
            if not current:
                raise ValueError(f"continuation line with no field: {line!r}")
            field, value = current[-1]
            current[-1] = (field, value + "\n" + line)
        else:
            field, sep, value = line.partition(":")
            if not sep:
                raise ValueError(f"not a field: {line!r}")
            current.append((field, value.strip()))
    if current:
        stanzas.append(current)
    return stanzas


def format_stanza(stanza: Stanza) -> str:
    return "".join(f"{field}: {value}\n" for field, value in stanza)


def _ar_members(data: bytes) -> Iterator[tuple[str, bytes]]:
    if not data.startswith(b"!<arch>\n"):
        raise ValueError("not an ar archive")
    offset = 8
    while offset + 60 <= len(data):
        header = data[offset : offset + 60]
        name = header[:16].decode().strip().rstrip("/")
        size = int(header[48:58].decode().strip())
        start = offset + 60
        yield name, data[start : start + size]
        # Members are padded to an even length.
        offset = start + size + (size % 2)


def _decompress(name: str, data: bytes) -> bytes:
    if name.endswith(".gz"):
        return gzip.decompress(data)
    if name.endswith(".xz"):
        return lzma.decompress(data)
    if name.endswith(".zst"):
        # Streamed, since the frame needn't say how big it is.
        with zstandard.ZstdDecompressor().stream_reader(data) as reader:
            return reader.read()
    if name.endswith(".tar"):
        return data
    raise ValueError(f"unknown compression: {name}")


def read_control(path: Path) -> Stanza:
    """The control stanza of the package at |path|."""
    for name, member in _ar_members(path.read_bytes()):
        if not name.startswith("control.tar"):
            continue
        with tarfile.open(fileobj=io.BytesIO(_decompress(name, member))) as tar:
            for info in tar.getmembers():
                if info.name in ("control", "./control"):
                    extracted = tar.extractfile(info)
                    if extracted is None:
                        break
                    stanzas = parse(extracted.read().decode())
                    if len(stanzas) != 1:
                        raise ValueError(f"{path}: expected one control stanza")
                    return stanzas[0]
        raise ValueError(f"{path}: no control file in {name}")
    raise ValueError(f"{path}: no control.tar")
