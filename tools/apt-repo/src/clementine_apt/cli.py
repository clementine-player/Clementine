"""clementine-apt: publishes .debs as a signed apt repository.

clementine-apt export-key --kms-key <version> --created <time> --out <dir>
clementine-apt publish --root <dir> --kms-key <version> --created <time>
    [--keep N] [--pruned <file>] <package.deb>...
"""

from __future__ import annotations

import argparse
import datetime
import sys
from pathlib import Path

from clementine_apt.openpgp import Key, armor
from clementine_apt.repo import Repository
from clementine_apt.signers import KmsSigner, LocalSigner, Signer

USER_ID = "Clementine archive signing key"
KEYRING = "clementine-archive-keyring"


def _timestamp(value: str) -> int:
    """Seconds since the epoch, or an ISO 8601 time with its time zone."""
    if value.isdigit():
        return int(value)
    when = datetime.datetime.fromisoformat(value)
    if when.tzinfo is None:
        raise argparse.ArgumentTypeError(f"{value} has no time zone")
    return int(when.timestamp())


def _add_key_arguments(parser: argparse.ArgumentParser) -> None:
    which = parser.add_mutually_exclusive_group(required=True)
    which.add_argument(
        "--kms-key",
        metavar="VERSION",
        help="the Cloud KMS key version: projects/…/cryptoKeyVersions/N",
    )
    which.add_argument(
        "--local-key",
        type=Path,
        metavar="PEM",
        help="an RSA private key file instead, for trying things out",
    )
    parser.add_argument(
        "--created",
        type=_timestamp,
        required=True,
        help="the key's creation time, which its fingerprint depends on: "
        "always the same one for the same key",
    )
    parser.add_argument(
        "--expect-fingerprint",
        metavar="HEX",
        help="stop unless the key has this fingerprint",
    )


def _key(args: argparse.Namespace) -> Key:
    signer: Signer = (
        KmsSigner(args.kms_key)
        if args.kms_key
        else LocalSigner.from_pem(args.local_key.read_bytes())
    )
    key = Key(signer, args.created, USER_ID)
    fingerprint = key.fingerprint.hex().upper()
    expected = (args.expect_fingerprint or "").replace(" ", "").upper()
    if expected and fingerprint != expected:
        raise SystemExit(f"The key's fingerprint is {fingerprint}, not {expected}")
    return key


def _write_public_key(key: Key, directory: Path) -> None:
    directory.mkdir(parents=True, exist_ok=True)
    public = key.public_key()
    (directory / f"{KEYRING}.gpg").write_bytes(public)
    (directory / f"{KEYRING}.asc").write_text(armor("PGP PUBLIC KEY BLOCK", public))


def export_key(args: argparse.Namespace) -> None:
    key = _key(args)
    _write_public_key(key, args.out)
    print(key.fingerprint.hex().upper())


def publish(args: argparse.Namespace) -> None:
    key = _key(args)
    repo = Repository(args.root)
    for path in args.packages:
        suite = repo.add(path)
        print(f"Added {path.name} to {suite}", file=sys.stderr)
    pruned = repo.prune(args.keep)
    for filename in pruned:
        print(f"Pruned {filename}", file=sys.stderr)
    if args.pruned:
        args.pruned.write_text("".join(f"{f}\n" for f in pruned))
    repo.write(key, datetime.datetime.now(datetime.UTC))
    _write_public_key(key, args.root)
    print(f"Signed with {key.fingerprint.hex().upper()}", file=sys.stderr)


def main(argv: list[str] | None = None) -> None:
    parser = argparse.ArgumentParser(prog="clementine-apt", description=__doc__)
    commands = parser.add_subparsers(dest="command", required=True)

    export = commands.add_parser("export-key", help="write out the public key")
    _add_key_arguments(export)
    export.add_argument("--out", type=Path, required=True, help="directory")
    export.set_defaults(run=export_key)

    pub = commands.add_parser("publish", help="add packages and re-sign")
    _add_key_arguments(pub)
    pub.add_argument("--root", type=Path, required=True, help="the repository")
    pub.add_argument(
        "--keep",
        type=int,
        default=10,
        help="versions of each package to keep in each suite (default 10)",
    )
    pub.add_argument(
        "--pruned",
        type=Path,
        help="write the pool files that are no longer needed here, one a line",
    )
    pub.add_argument("packages", type=Path, nargs="*", metavar="DEB")
    pub.set_defaults(run=publish)

    args = parser.parse_args(argv)
    args.run(args)


if __name__ == "__main__":
    main()
