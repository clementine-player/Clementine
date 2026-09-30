"""Just enough OpenPGP (RFC 4880, v4) to publish an RSA key and sign with it,
where the key itself is somewhere we can only ask for signatures.

GnuPG can't use a Cloud KMS key, so this builds the packets itself around
the raw PKCS#1 v1.5 signatures a Signer makes. v4 packets and SHA-256 are
what every verifier apt uses (gpgv, and sqv from Debian 13 and Ubuntu 25.04)
accepts.
"""

from __future__ import annotations

import base64
import hashlib

from clementine_apt.signers import Signer

_RSA = 1
_SHA256 = 8

_TAG_SIGNATURE = 2
_TAG_PUBLIC_KEY = 6
_TAG_USER_ID = 13

_SIG_BINARY = 0x00
_SIG_TEXT = 0x01
_SIG_POSITIVE_CERTIFICATION = 0x13

_SUB_CREATION_TIME = 2
_SUB_ISSUER = 16
_SUB_PREFERRED_HASHES = 21
_SUB_KEY_FLAGS = 27
_SUB_ISSUER_FINGERPRINT = 33

_KEY_FLAG_CERTIFY = 0x01
_KEY_FLAG_SIGN = 0x02


def _packet(tag: int, body: bytes) -> bytes:
    """A new-format packet."""
    n = len(body)
    if n < 192:
        length = bytes([n])
    elif n < 8384:
        n -= 192
        length = bytes([(n >> 8) + 192, n & 0xFF])
    else:
        length = b"\xff" + n.to_bytes(4, "big")
    return bytes([0xC0 | tag]) + length + body


def _mpi(value: int) -> bytes:
    return value.bit_length().to_bytes(2, "big") + value.to_bytes(
        (value.bit_length() + 7) // 8, "big"
    )


def _subpacket(kind: int, data: bytes) -> bytes:
    length = len(data) + 1
    if length >= 192:
        raise ValueError("subpacket too long")
    return bytes([length, kind]) + data


def _crc24(data: bytes) -> int:
    crc = 0xB704CE
    for byte in data:
        crc ^= byte << 16
        for _ in range(8):
            crc <<= 1
            if crc & 0x1000000:
                crc ^= 0x1864CFB
    return crc & 0xFFFFFF


def armor(kind: str, data: bytes) -> str:
    """ASCII armor, e.g. kind "PGP SIGNATURE"."""
    encoded = base64.b64encode(data).decode()
    lines = [encoded[i : i + 64] for i in range(0, len(encoded), 64)]
    checksum = base64.b64encode(_crc24(data).to_bytes(3, "big")).decode()
    return (
        f"-----BEGIN {kind}-----\n\n"
        + "\n".join(lines)
        + f"\n={checksum}\n-----END {kind}-----\n"
    )


def _canonical_text(text: str) -> bytes:
    """What a text signature covers: CRLF line endings, with the whitespace at
    the end of each line removed."""
    return "\r\n".join(line.rstrip(" \t") for line in text.split("\n")).encode()


class Key:
    """An RSA signing key, as OpenPGP sees it.

    Its fingerprint depends on |created|, so the same key published with a
    different creation time is a different OpenPGP key: keep it fixed.
    """

    def __init__(self, signer: Signer, created: int, user_id: str) -> None:
        self._signer = signer
        self.created = created
        self.user_id = user_id
        numbers = signer.public_key().public_numbers()
        self._body = (
            bytes([4])
            + created.to_bytes(4, "big")
            + bytes([_RSA])
            + _mpi(numbers.n)
            + _mpi(numbers.e)
        )
        self.fingerprint = hashlib.sha1(self._hashed_key()).digest()
        self.key_id = self.fingerprint[-8:]

    def _hashed_key(self) -> bytes:
        return b"\x99" + len(self._body).to_bytes(2, "big") + self._body

    def _signature(
        self, kind: int, signed: bytes, created: int, extra: bytes = b""
    ) -> bytes:
        hashed = (
            _subpacket(_SUB_CREATION_TIME, created.to_bytes(4, "big"))
            + _subpacket(_SUB_ISSUER_FINGERPRINT, b"\x04" + self.fingerprint)
            + extra
        )
        header = bytes([4, kind, _RSA, _SHA256]) + len(hashed).to_bytes(2, "big")
        header += hashed
        trailer = b"\x04\xff" + len(header).to_bytes(4, "big")
        digest = hashlib.sha256(signed + header + trailer).digest()
        signature = self._signer.sign_digest(digest)
        unhashed = _subpacket(_SUB_ISSUER, self.key_id)
        body = (
            header
            + len(unhashed).to_bytes(2, "big")
            + unhashed
            + digest[:2]
            + _mpi(int.from_bytes(signature, "big"))
        )
        return _packet(_TAG_SIGNATURE, body)

    def public_key(self) -> bytes:
        """The key as a keyring file: the key, its user ID, and its own
        signature binding the two. Signed as of |created|, so it comes out
        the same every time."""
        user_id = self.user_id.encode()
        certification = self._signature(
            _SIG_POSITIVE_CERTIFICATION,
            self._hashed_key() + b"\xb4" + len(user_id).to_bytes(4, "big") + user_id,
            self.created,
            _subpacket(_SUB_KEY_FLAGS, bytes([_KEY_FLAG_CERTIFY | _KEY_FLAG_SIGN]))
            + _subpacket(_SUB_PREFERRED_HASHES, bytes([_SHA256])),
        )
        return (
            _packet(_TAG_PUBLIC_KEY, self._body)
            + _packet(_TAG_USER_ID, user_id)
            + certification
        )

    def sign_detached(self, data: bytes, now: int) -> str:
        """An armored detached signature of |data|, as Release.gpg is."""
        return armor("PGP SIGNATURE", self._signature(_SIG_BINARY, data, now))

    def sign_cleartext(self, text: str, now: int) -> str:
        """|text| with its signature around it, as InRelease is."""
        if not text.endswith("\n"):
            text += "\n"
        # The line break before the signature belongs to the armor.
        signed = _canonical_text(text[:-1])
        signature = self._signature(_SIG_TEXT, signed, now)
        # Lines starting with a dash would read as armor.
        escaped = "".join(
            ("- " + line if line.startswith("-") else line)
            for line in text.splitlines(keepends=True)
        )
        return (
            "-----BEGIN PGP SIGNED MESSAGE-----\nHash: SHA256\n\n"
            + escaped
            + armor("PGP SIGNATURE", signature)
        )
