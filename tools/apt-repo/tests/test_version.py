from __future__ import annotations

import pytest

from clementine_apt.version import compare


@pytest.mark.parametrize(
    ("older", "newer"),
    [
        ("1.0", "1.1"),
        ("1.9", "1.10"),
        ("1.0~rc1", "1.0"),
        ("1.0", "1.0a"),
        ("1.0-1", "1.0-2"),
        ("1:0.1", "2:0.0"),
        ("0:9.9", "1:0.0"),
        ("1.0a", "1.0+a"),
        # Clementine's own: git describe, then the suite.
        ("1.4.1-243-g790ed6bef~noble", "1.4.1-251-ga588bb3f8~noble"),
        ("1.4.1-99-gffffffff~noble", "1.4.1-100-g00000000~noble"),
        ("1.4.1~noble", "1.4.1-1-g1234567~noble"),
    ],
)
def test_order(older: str, newer: str) -> None:
    assert compare(older, newer) < 0
    assert compare(newer, older) > 0


@pytest.mark.parametrize(
    ("a", "b"), [("1.0", "1.0"), ("1.00", "1.0"), ("0:1.0", "1.0")]
)
def test_equal(a: str, b: str) -> None:
    assert compare(a, b) == 0
