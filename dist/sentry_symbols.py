#!/usr/bin/env python3
"""Checks and uploads the debug symbols Sentry needs for Clementine's crashes.

Crash reports from the macOS and Windows builds (see src/core/crashreporting.h)
are minidumps: addresses, not function names. Sentry turns them into names and
lines with debug files matched by debug ID: a dSYM for each Mach-O executable,
a PDB for each .exe. all.yml collects them into a directory for each build.

Usage:

  uv run --only-group sentry dist/sentry_symbols.py \\
      check SYMBOLS BINARY...

    Fails unless every BINARY, as shipped, has a file in SYMBOLS with debug
    info and the same debug ID, so a build can't quietly stop making them.

  uv run --only-group sentry dist/sentry_symbols.py \\
      upload SYMBOLS

    Uploads SYMBOLS to Sentry, with sentry-cli's SENTRY_AUTH_TOKEN, SENTRY_ORG
    and SENTRY_PROJECT. Without a token or a project it only says so, so
    builds without the repository's secrets still pass.
"""

import argparse
import json
import os
import pathlib
import subprocess
import sys
from collections.abc import Iterable
from dataclasses import dataclass


@dataclass(frozen=True)
class DebugFile:
    """What sentry-cli says about one file."""

    path: str
    debug_ids: frozenset[str]
    has_debug_info: bool


def parse_check(path: str, output: str) -> DebugFile:
    """Reads `sentry-cli debug-files check --json` output for a file."""
    data = json.loads(output)
    features = {f.strip() for f in (data.get("features") or "").split(",")}
    return DebugFile(
        path=path,
        debug_ids=frozenset(v["debug_id"] for v in data.get("variants", [])),
        has_debug_info=bool(data.get("is_usable")) and "debug" in features,
    )


def check_file(path: pathlib.Path) -> DebugFile | None:
    """Asks sentry-cli about a file, or None if it isn't a debug file."""
    result = subprocess.run(
        ["sentry-cli", "debug-files", "check", "--json", str(path)],
        capture_output=True,
        text=True,
        check=False,
    )
    if result.returncode != 0:
        return None
    return parse_check(str(path), result.stdout)


def missing_symbols(
    binaries: Iterable[DebugFile], symbols: Iterable[DebugFile]
) -> list[str]:
    """The problems with symbols for binaries, or none if each has them."""
    with_debug_info = set()
    for symbol in symbols:
        if symbol.has_debug_info:
            with_debug_info |= symbol.debug_ids
    problems = []
    for binary in binaries:
        if not binary.debug_ids:
            problems.append(f"{binary.path} has no debug ID")
        for debug_id in sorted(binary.debug_ids - with_debug_info):
            problems.append(f"{binary.path}: no debug info for {debug_id}")
    return problems


def files_under(directory: pathlib.Path) -> list[pathlib.Path]:
    return sorted(p for p in directory.rglob("*") if p.is_file())


def check(symbols_dir: pathlib.Path, binary_paths: list[pathlib.Path]) -> int:
    binaries = []
    for path in binary_paths:
        binary = check_file(path)
        if binary is None:
            print(f"::error::{path} isn't something Sentry can read")
            return 1
        binaries.append(binary)
    symbols = [f for f in map(check_file, files_under(symbols_dir)) if f]
    for symbol in symbols:
        kind = "debug info" if symbol.has_debug_info else "no debug info"
        print(f"{symbol.path}: {', '.join(sorted(symbol.debug_ids))} ({kind})")
    problems = missing_symbols(binaries, symbols)
    for problem in problems:
        print(f"::error::{problem}")
    return 1 if problems else 0


def upload(symbols_dir: pathlib.Path) -> int:
    missing = [
        name
        for name in ("SENTRY_AUTH_TOKEN", "SENTRY_PROJECT")
        if not os.environ.get(name)
    ]
    if missing:
        print(
            f"::notice::Not uploading debug symbols to Sentry: no "
            f"{' or '.join(missing)}."
        )
        return 0
    return subprocess.run(
        ["sentry-cli", "debug-files", "upload", str(symbols_dir)], check=False
    ).returncode


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    commands = parser.add_subparsers(dest="command", required=True)
    check_parser = commands.add_parser("check")
    check_parser.add_argument("symbols", type=pathlib.Path)
    check_parser.add_argument("binaries", type=pathlib.Path, nargs="+")
    upload_parser = commands.add_parser("upload")
    upload_parser.add_argument("symbols", type=pathlib.Path)
    args = parser.parse_args(argv)
    if args.command == "check":
        return check(args.symbols, args.binaries)
    return upload(args.symbols)


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
