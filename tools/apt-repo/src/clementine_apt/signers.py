"""Things that make RSA signatures: Cloud KMS, or a local key for tests."""

from __future__ import annotations

import base64
from typing import Protocol

import google.auth
from cryptography.exceptions import InvalidSignature
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import padding, rsa
from cryptography.hazmat.primitives.asymmetric.utils import Prehashed
from google.auth.transport.requests import AuthorizedSession

KMS_API = "https://cloudkms.googleapis.com/v1"
# PKCS#1 v1.5 over SHA-256 is what OpenPGP's RSA signatures are.
KMS_ALGORITHM = "RSA_SIGN_PKCS1_4096_SHA256"


class Signer(Protocol):
    """Signs SHA-256 digests with RSA, PKCS#1 v1.5."""

    def public_key(self) -> rsa.RSAPublicKey: ...

    def sign_digest(self, digest: bytes) -> bytes: ...


class LocalSigner:
    """A private key in memory. For tests and trying things out: the
    published repository is only ever signed in Cloud KMS."""

    def __init__(self, key: rsa.RSAPrivateKey) -> None:
        self._key = key

    @classmethod
    def generate(cls, bits: int = 2048) -> LocalSigner:
        return cls(rsa.generate_private_key(public_exponent=65537, key_size=bits))

    @classmethod
    def from_pem(cls, pem: bytes) -> LocalSigner:
        key = serialization.load_pem_private_key(pem, password=None)
        if not isinstance(key, rsa.RSAPrivateKey):
            raise ValueError("not an RSA private key")
        return cls(key)

    def public_key(self) -> rsa.RSAPublicKey:
        return self._key.public_key()

    def sign_digest(self, digest: bytes) -> bytes:
        return self._key.sign(digest, padding.PKCS1v15(), Prehashed(hashes.SHA256()))


class KmsSigner:
    """A key version in Cloud KMS, which never leaves it: KMS signs each
    digest. Authenticates with Application Default Credentials."""

    def __init__(self, key_version: str) -> None:
        credentials, _ = google.auth.default(
            scopes=["https://www.googleapis.com/auth/cloudkms"]
        )
        self._session = AuthorizedSession(credentials)
        self._key_version = key_version

        response = self._session.get(f"{KMS_API}/{key_version}/publicKey")
        response.raise_for_status()
        body = response.json()
        if body.get("algorithm") != KMS_ALGORITHM:
            raise ValueError(
                f"{key_version} is {body.get('algorithm')}, not {KMS_ALGORITHM}"
            )
        key = serialization.load_pem_public_key(body["pem"].encode())
        if not isinstance(key, rsa.RSAPublicKey):
            raise ValueError(f"{key_version} isn't an RSA key")
        self._public_key = key

    def public_key(self) -> rsa.RSAPublicKey:
        return self._public_key

    def sign_digest(self, digest: bytes) -> bytes:
        response = self._session.post(
            f"{KMS_API}/{self._key_version}:asymmetricSign",
            json={"digest": {"sha256": base64.b64encode(digest).decode()}},
        )
        response.raise_for_status()
        signature = base64.b64decode(response.json()["signature"])
        # Checked here rather than by the first apt client to trip over it.
        try:
            self._public_key.verify(
                signature, digest, padding.PKCS1v15(), Prehashed(hashes.SHA256())
            )
        except InvalidSignature as e:
            raise ValueError(
                "Cloud KMS returned a signature that doesn't verify"
            ) from e
        return signature
