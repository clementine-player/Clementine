"""Puts a signed DMG in the Sparkle feed.

The feed (data.clementine-player.org's /sparkle2) lists the MacUpdate
entities in the clementine-data project's Datastore. Everything an entry says
about the build is read from the app in the DMG, so it can't disagree with
what users install: Sparkle compares sparkle:version with the
installed app's CFBundleVersion, and only offers an update to a macOS at least
its LSMinimumSystemVersion.

Before publishing, it checks the signature with the public key in the app's
own Info.plist, the one the builds after it check their updates with: an
update signed with any other key would fail the next update's check.
"""

from __future__ import annotations

import base64
import datetime
import plistlib
import re
import subprocess
import tempfile
from dataclasses import dataclass
from pathlib import Path

from cryptography.exceptions import InvalidSignature

# hazmat is where cryptography keeps all its primitives, Ed25519 included. Its
# warning is for those with choices to get wrong (modes, padding, nonces);
# Ed25519 verification has none. sign_update --verify can't do this check: it
# derives the public key from the private one, not the key in the app.
from cryptography.hazmat.primitives.asymmetric.ed25519 import Ed25519PublicKey
from google.cloud import datastore
from google.cloud.datastore.query import PropertyFilter

PROJECT = "clementine-data"
KIND = "MacUpdate"
# What the feed queries by: the newest update for each minimum macOS.
INDEXED = ("min_macos", "published")
FEED_URL = "https://data.clementine-player.org/sparkle2"


@dataclass(frozen=True)
class App:
    """What the feed needs from the app's Info.plist."""

    version: str  # CFBundleShortVersionString: what people see
    build: str  # CFBundleVersion: what Sparkle compares
    min_macos: str
    feed_url: str
    public_key: str

    @classmethod
    def from_info_plist(cls, info: dict[str, object]) -> App:
        def string(key: str) -> str:
            value = info.get(key)
            if not isinstance(value, str):
                raise SystemExit(f"The app's Info.plist has no {key}.")
            return value

        return cls(
            version=string("CFBundleShortVersionString"),
            build=string("CFBundleVersion"),
            min_macos=string("LSMinimumSystemVersion"),
            feed_url=string("SUFeedURL"),
            public_key=string("SUPublicEDKey"),
        )


def read_app(dmg: Path) -> App:
    """Reads the Info.plist of the app in a DMG. macOS only: it mounts it."""
    with tempfile.TemporaryDirectory() as mountpoint:
        subprocess.run(
            [
                "hdiutil",
                "attach",
                "-nobrowse",
                "-readonly",
                "-noautoopen",
                "-mountpoint",
                mountpoint,
                str(dmg),
            ],
            check=True,
            stdout=subprocess.DEVNULL,
        )
        try:
            apps = list(Path(mountpoint).glob("*.app"))
            if len(apps) != 1:
                raise SystemExit(f"Expected one app in {dmg}, found {apps}")
            with (apps[0] / "Contents" / "Info.plist").open("rb") as f:
                return App.from_info_plist(plistlib.load(f))
        finally:
            subprocess.run(
                ["hdiutil", "detach", mountpoint],
                check=True,
                stdout=subprocess.DEVNULL,
            )


def check(app: App, dmg: Path, signature: str) -> None:
    """Stops unless installs of this app would take this DMG as its update."""
    if app.feed_url != FEED_URL:
        raise SystemExit(f"The app checks {app.feed_url} for updates, not {FEED_URL}.")
    if not app.public_key:
        raise SystemExit("The app has no SUPublicEDKey: it was built without Sparkle.")
    # The feed picks the highest build for each macOS, comparing them number by
    # number as Sparkle does. cmake/Version.cmake always makes such builds, but
    # a tag with letters in it (1.5rc1) would put them in this one.
    if not re.fullmatch(r"\d+(\.\d+)*", app.build):
        raise SystemExit(
            f"The app's CFBundleVersion, {app.build}, isn't only dotted numbers."
        )
    key = Ed25519PublicKey.from_public_bytes(base64.b64decode(app.public_key))
    try:
        key.verify(base64.b64decode(signature), dmg.read_bytes())
    except InvalidSignature:
        raise SystemExit(
            f"The signature doesn't match the app's SUPublicEDKey, {app.public_key}."
        ) from None


def entry(
    app: App, dmg: Path, url: str, signature: str, notes: list[str]
) -> dict[str, object]:
    """The MacUpdate entity's properties (models.py in the Website repository)."""
    return {
        "version": app.version,
        "build": app.build,
        "min_macos": app.min_macos,
        "download_url": url,
        "ed_signature": signature,
        "length": dmg.stat().st_size,
        "notes": notes,
        "published": datetime.datetime.now(datetime.UTC),
    }


def build_key(build: str) -> tuple[int, ...]:
    """How Sparkle compares builds of dotted numbers: number by number, with
    any missing at the end counting as 0."""
    parts = [int(part) for part in build.split(".")]
    while parts and parts[-1] == 0:
        parts.pop()
    return tuple(parts)


def newest_build(client: datastore.Client, min_macos: str) -> str | None:
    """The most recently published build for a minimum macOS, if any."""
    query = client.query(kind=KIND)
    query.add_filter(filter=PropertyFilter("min_macos", "=", min_macos))
    query.order = ["-published"]
    newest = list(query.fetch(limit=1))
    return str(newest[0]["build"]) if newest else None


def put(properties: dict[str, object]) -> str:
    """Publishes the update, and returns its key's name: its build.

    Sparkle drops the updates the Mac's macOS can't run, then offers the
    highest build of the rest. The feed finds that by taking the most recently
    published update for each minimum macOS, so this keeps the most recent
    one the highest: it stops rather than publish a lower build than that.
    Publishing the same build again replaces it, with the new notes.

    Nothing else writes these, and the workflow publishes one at a time, so
    there's no transaction around the check and the write.
    """
    build = str(properties["build"])
    min_macos = str(properties["min_macos"])
    client = datastore.Client(project=PROJECT)
    newest = newest_build(client, min_macos)
    if newest is not None and build_key(newest) > build_key(build):
        raise SystemExit(
            f"The feed has a higher build for macOS {min_macos}, {newest},"
            f" than {build}."
        )
    entity = datastore.Entity(
        client.key(KIND, build),
        exclude_from_indexes=tuple(p for p in properties if p not in INDEXED),
    )
    entity.update(properties)
    client.put(entity)
    return build
