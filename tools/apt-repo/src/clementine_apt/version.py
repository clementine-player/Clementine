"""Debian version comparison, as dpkg does it."""

from __future__ import annotations

import functools


def _order(c: str) -> int:
    # The end of the string sorts as 0, like a digit; ~ before anything, even
    # the end; letters before everything else.
    if not c or c.isdigit():
        return 0
    if c.isalpha():
        return ord(c)
    if c == "~":
        return -1
    return ord(c) + 256


def _compare_part(a: str, b: str) -> int:
    """dpkg's verrevcmp: runs of non-digits, then runs of digits, in turn."""
    i = j = 0
    while i < len(a) or j < len(b):
        while (i < len(a) and not a[i].isdigit()) or (
            j < len(b) and not b[j].isdigit()
        ):
            ac = _order(a[i] if i < len(a) else "")
            bc = _order(b[j] if j < len(b) else "")
            if ac != bc:
                return ac - bc
            i += 1
            j += 1
        while i < len(a) and a[i] == "0":
            i += 1
        while j < len(b) and b[j] == "0":
            j += 1
        first_diff = 0
        while i < len(a) and a[i].isdigit() and j < len(b) and b[j].isdigit():
            if not first_diff:
                first_diff = ord(a[i]) - ord(b[j])
            i += 1
            j += 1
        if i < len(a) and a[i].isdigit():
            return 1
        if j < len(b) and b[j].isdigit():
            return -1
        if first_diff:
            return first_diff
    return 0


def _split(version: str) -> tuple[int, str, str]:
    epoch = 0
    if ":" in version:
        head, version = version.split(":", 1)
        epoch = int(head)
    upstream, _, revision = version.rpartition("-")
    if not upstream:
        return epoch, revision, ""
    return epoch, upstream, revision


def compare(a: str, b: str) -> int:
    """Negative, zero or positive as |a| is older than, the same as or newer
    than |b|."""
    ea, ua, ra = _split(a)
    eb, ub, rb = _split(b)
    if ea != eb:
        return ea - eb
    return _compare_part(ua, ub) or _compare_part(ra, rb)


sort_key = functools.cmp_to_key(compare)
