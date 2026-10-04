from __future__ import annotations

import base64
import datetime
from pathlib import Path

import pytest
from cryptography.hazmat.primitives.asymmetric.ed25519 import Ed25519PrivateKey
from cryptography.hazmat.primitives.serialization import Encoding, PublicFormat
from google.cloud import datastore
from google.cloud.datastore.query import PropertyFilter

from clementine_mac_update import publish


def _public_key(key: Ed25519PrivateKey) -> str:
    raw = key.public_key().public_bytes(Encoding.Raw, PublicFormat.Raw)
    return base64.b64encode(raw).decode()


def _app(
    public_key: str,
    feed_url: str = publish.FEED_URL,
    build: str = "4096.1.4.1.2.300",
) -> publish.App:
    return publish.App.from_info_plist(
        {
            "CFBundleShortVersionString": "1.4.1-300-gabcdef",
            "CFBundleVersion": build,
            "LSMinimumSystemVersion": "26.0",
            "SUFeedURL": feed_url,
            "SUPublicEDKey": public_key,
        }
    )


@pytest.fixture
def dmg(tmp_path: Path) -> Path:
    path = tmp_path / "clementine.dmg"
    path.write_bytes(b"a disk image")
    return path


def _sign(key: Ed25519PrivateKey, dmg: Path) -> str:
    return base64.b64encode(key.sign(dmg.read_bytes())).decode()


def test_accepts_a_dmg_signed_with_the_apps_key(dmg: Path) -> None:
    key = Ed25519PrivateKey.generate()
    publish.check(_app(_public_key(key)), dmg, _sign(key, dmg))


def test_refuses_a_dmg_signed_with_another_key(dmg: Path) -> None:
    app = _app(_public_key(Ed25519PrivateKey.generate()))
    with pytest.raises(SystemExit, match="doesn't match"):
        publish.check(app, dmg, _sign(Ed25519PrivateKey.generate(), dmg))


def test_refuses_an_app_that_checks_another_feed(dmg: Path) -> None:
    key = Ed25519PrivateKey.generate()
    app = _app(_public_key(key), feed_url="https://example.com/sparkle")
    with pytest.raises(SystemExit, match="https://example"):
        publish.check(app, dmg, _sign(key, dmg))


def test_refuses_an_app_built_without_sparkle(dmg: Path) -> None:
    key = Ed25519PrivateKey.generate()
    with pytest.raises(SystemExit, match="without Sparkle"):
        publish.check(_app(""), dmg, _sign(key, dmg))


def test_refuses_a_build_the_feed_cant_compare(dmg: Path) -> None:
    key = Ed25519PrivateKey.generate()
    app = _app(_public_key(key), build="4096.1.5rc1.2.3")
    with pytest.raises(SystemExit, match="dotted numbers"):
        publish.check(app, dmg, _sign(key, dmg))


def test_refuses_an_info_plist_without_a_key() -> None:
    with pytest.raises(SystemExit, match="LSMinimumSystemVersion"):
        publish.App.from_info_plist(
            {"CFBundleShortVersionString": "1.4.1", "CFBundleVersion": "4096.1.4.1"}
        )


def test_entry_describes_the_build_from_its_app(dmg: Path) -> None:
    key = Ed25519PrivateKey.generate()
    entry = publish.entry(
        _app(_public_key(key)), dmg, "https://example.com/c.dmg", "c2ln", ["A & B"]
    )
    published = entry.pop("published")
    assert isinstance(published, datetime.datetime)
    assert published.tzinfo is not None
    assert entry == {
        "version": "1.4.1-300-gabcdef",
        "build": "4096.1.4.1.2.300",
        "min_macos": "26.0",
        "download_url": "https://example.com/c.dmg",
        "ed_signature": "c2ln",
        "length": len(b"a disk image"),
        "notes": ["A & B"],
    }


class FakeQuery:
    """The queries newest_build makes: one equality filter, one order."""

    def __init__(self, entities: list[datastore.Entity]) -> None:
        self.entities = entities
        self.filters: list[PropertyFilter] = []
        self.order: list[str] = []

    def add_filter(self, *, filter: PropertyFilter) -> None:
        assert filter.operator == "="
        self.filters.append(filter)

    def fetch(self, limit: int) -> list[datastore.Entity]:
        found = [
            e
            for e in self.entities
            if all(e[f.property_name] == f.value for f in self.filters)
        ]
        for order in reversed(self.order):
            name = order.removeprefix("-")
            found.sort(key=lambda e: e[name], reverse=order.startswith("-"))
        return found[:limit]


class FakeClient:
    """Datastore, in memory."""

    def __init__(self, project: str) -> None:
        self.project = project
        self.entities: dict[tuple[str, str], datastore.Entity] = {}

    def key(self, kind: str, name: str) -> datastore.Key:
        return datastore.Key(kind, name, project=self.project)

    def query(self, kind: str) -> FakeQuery:
        return FakeQuery([e for (k, _), e in self.entities.items() if k == kind])

    def put(self, entity: datastore.Entity) -> None:
        assert entity.key is not None
        self.entities[(entity.key.kind, entity.key.name)] = entity


@pytest.fixture
def store(monkeypatch: pytest.MonkeyPatch) -> FakeClient:
    client = FakeClient("clementine-data")

    def connect(project: str) -> FakeClient:
        assert project == "clementine-data"
        return client

    monkeypatch.setattr(datastore, "Client", connect)
    return client


def _publish(dmg: Path, build: str, notes: list[str]) -> str:
    key = Ed25519PrivateKey.generate()
    app = _app(_public_key(key), build=build)
    return publish.put(publish.entry(app, dmg, "https://e/c.dmg", "c2ln", notes))


def _stored(store: FakeClient) -> dict[str, tuple[object, object]]:
    """Each entity's name: its build and notes."""
    return {
        name: (entity["build"], entity["notes"])
        for (_, name), entity in store.entities.items()
    }


def test_put_names_the_update_after_its_build(dmg: Path, store: FakeClient) -> None:
    assert _publish(dmg, "4096.1.4.1.2.300", ["New"]) == "4096.1.4.1.2.300"
    [((kind, _), entity)] = store.entities.items()
    assert kind == "MacUpdate"
    assert entity["min_macos"] == "26.0"
    assert entity.exclude_from_indexes == set(entity) - {"min_macos", "published"}


def test_put_keeps_every_update(dmg: Path, store: FakeClient) -> None:
    _publish(dmg, "4096.1.4.1.2.300", ["Old"])
    # Only higher as a number, not as text.
    _publish(dmg, "4096.1.4.1.2.1000", ["New"])
    assert _stored(store) == {
        "4096.1.4.1.2.300": ("4096.1.4.1.2.300", ["Old"]),
        "4096.1.4.1.2.1000": ("4096.1.4.1.2.1000", ["New"]),
    }


def test_put_replaces_the_same_build(dmg: Path, store: FakeClient) -> None:
    _publish(dmg, "4096.1.4.1.2.300", ["Old"])
    _publish(dmg, "4096.1.4.1.2.300", ["Fixed note"])
    assert _stored(store) == {"4096.1.4.1.2.300": ("4096.1.4.1.2.300", ["Fixed note"])}


def test_put_refuses_a_lower_build(dmg: Path, store: FakeClient) -> None:
    _publish(dmg, "4096.1.4.1.2.1000", ["Newest"])
    with pytest.raises(SystemExit, match="higher build"):
        _publish(dmg, "4096.1.4.1.2.300", ["Older"])
    assert _stored(store) == {"4096.1.4.1.2.1000": ("4096.1.4.1.2.1000", ["Newest"])}


def test_put_compares_with_the_most_recent_update(dmg: Path, store: FakeClient) -> None:
    _publish(dmg, "4096.1.4.1.2.300", ["Old"])
    _publish(dmg, "4096.1.4.1.2.1000", ["Newest"])
    with pytest.raises(SystemExit, match="1000"):
        _publish(dmg, "4096.1.4.1.2.500", ["Between"])


def test_builds_compare_as_sparkle_compares_them() -> None:
    assert publish.build_key("4096.1.4.1.2.1000") > publish.build_key(
        "4096.1.4.1.2.300"
    )
    assert publish.build_key("4096.1.4.1") == publish.build_key("4096.1.4.1.0")
    assert publish.build_key("4096.1.4.1.0.5") > publish.build_key("4096.1.4.1")
