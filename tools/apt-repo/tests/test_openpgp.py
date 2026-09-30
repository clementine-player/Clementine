from __future__ import annotations

import subprocess
from pathlib import Path

from conftest import verify

from clementine_apt.openpgp import Key


def test_public_key_is_what_gpg_reads(key: Key, keyring: Path) -> None:
    listing = subprocess.run(
        ["gpg", "--with-colons", "--show-keys", str(keyring)],
        check=True,
        capture_output=True,
        text=True,
    ).stdout
    records = [line.split(":") for line in listing.splitlines()]
    fingerprints = [r[9] for r in records if r[0] == "fpr"]
    assert fingerprints == [key.fingerprint.hex().upper()]
    uids = [r[9] for r in records if r[0] == "uid"]
    assert uids == ["Clementine test key"]
    # Usable for signing, and the self-signature checked out.
    pub = next(r for r in records if r[0] == "pub")
    assert "s" in pub[11]
    assert pub[1] not in ("i", "r", "e")


def test_public_key_is_the_same_every_time(key: Key) -> None:
    assert key.public_key() == key.public_key()


def test_detached_signature(key: Key, keyring: Path, tmp_path: Path) -> None:
    data = tmp_path / "Release"
    data.write_bytes(b"Origin: Clementine\nSuite: noble\n")
    signature = tmp_path / "Release.gpg"
    signature.write_text(key.sign_detached(data.read_bytes(), 1_790_000_100))
    verify(keyring, signature, data)


def test_cleartext_signature(key: Key, keyring: Path, tmp_path: Path) -> None:
    # Trailing spaces, and a line starting with a dash, which is escaped.
    text = "Origin: Clementine\nDescription: trailing   \n-dash\nSHA256:\n x 1 y\n"
    signed = tmp_path / "InRelease"
    signed.write_text(key.sign_cleartext(text, 1_790_000_100))
    verify(keyring, signed)
    assert "\n- -dash\n" in signed.read_text()


def test_tampering_is_caught(key: Key, keyring: Path, tmp_path: Path) -> None:
    signed = tmp_path / "InRelease"
    signed.write_text(key.sign_cleartext("Suite: noble\n", 1_790_000_100))
    signed.write_text(signed.read_text().replace("noble", "jammy"))
    result = subprocess.run(
        ["gpgv", "--keyring", str(keyring), str(signed)], capture_output=True
    )
    assert result.returncode != 0
