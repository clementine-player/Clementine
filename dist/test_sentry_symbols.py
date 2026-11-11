import json

import pytest

import sentry_symbols
from sentry_symbols import DebugFile, library_summary, missing_symbols, parse_check


def check_output(debug_ids: list[str], features: str, usable: bool = True) -> str:
    return json.dumps(
        {
            "type": "pe",
            "variants": [{"debug_id": d, "arch": "x86_64"} for d in debug_ids],
            "features": features,
            "is_usable": usable,
        }
    )


def test_parse_check_debug_info() -> None:
    f = parse_check("a.pdb", check_output(["id1"], "symtab, debug"))
    assert f == DebugFile(
        "a.pdb", frozenset({"id1"}), True, frozenset({"symtab", "debug"})
    )


def test_parse_check_no_debug_info() -> None:
    f = parse_check("a.exe", check_output(["id1"], "symtab, unwind"))
    assert not f.has_debug_info


def test_parse_check_unusable() -> None:
    f = parse_check("a.pdb", check_output(["id1"], "debug", usable=False))
    assert not f.has_debug_info


def test_parse_check_no_features() -> None:
    output = json.dumps({"type": "pe", "variants": [], "features": None})
    assert parse_check("a.exe", output) == DebugFile(
        "a.exe", frozenset(), False, frozenset()
    )


def test_binaries_with_symbols() -> None:
    binaries = [DebugFile("a.exe", frozenset({"id1"}), False)]
    symbols = [
        DebugFile("a.exe", frozenset({"id1"}), False),
        DebugFile("a.pdb", frozenset({"id1"}), True),
    ]
    assert missing_symbols(binaries, symbols) == []


def test_symbols_for_another_build() -> None:
    binaries = [DebugFile("a.exe", frozenset({"id1"}), False)]
    symbols = [DebugFile("a.pdb", frozenset({"id2"}), True)]
    assert missing_symbols(binaries, symbols) == ["a.exe: no debug info for id1"]


def test_symbols_without_debug_info() -> None:
    binaries = [DebugFile("a", frozenset({"id1"}), False)]
    symbols = [DebugFile("a", frozenset({"id1"}), False)]
    assert missing_symbols(binaries, symbols) == ["a: no debug info for id1"]


def test_universal_binary_needs_every_architecture() -> None:
    binaries = [DebugFile("a", frozenset({"arm", "x86"}), False)]
    symbols = [DebugFile("a.dSYM/a", frozenset({"arm"}), True)]
    assert missing_symbols(binaries, symbols) == ["a: no debug info for x86"]


def test_binary_without_debug_id() -> None:
    binaries = [DebugFile("a", frozenset(), False)]
    assert missing_symbols(binaries, []) == ["a has no debug ID"]


@pytest.mark.parametrize(
    ("env", "missing"),
    [
        ({}, "SENTRY_AUTH_TOKEN or SENTRY_PROJECT"),
        ({"SENTRY_AUTH_TOKEN": "t"}, "SENTRY_PROJECT"),
        ({"SENTRY_PROJECT": "p", "SENTRY_AUTH_TOKEN": ""}, "SENTRY_AUTH_TOKEN"),
    ],
)
def test_upload_without_settings_only_notes_it(
    monkeypatch: pytest.MonkeyPatch,
    capsys: pytest.CaptureFixture[str],
    env: dict[str, str],
    missing: str,
) -> None:
    monkeypatch.delenv("SENTRY_AUTH_TOKEN", raising=False)
    monkeypatch.delenv("SENTRY_PROJECT", raising=False)
    for name, value in env.items():
        monkeypatch.setenv(name, value)

    def fail(*args: object, **kwargs: object) -> None:
        raise AssertionError("ran sentry-cli")

    monkeypatch.setattr(sentry_symbols.subprocess, "run", fail)
    assert sentry_symbols.main(["upload", "symbols"]) == 0
    assert f"no {missing}." in capsys.readouterr().out


def test_library_summary() -> None:
    libraries = [
        DebugFile(
            "Qt6Core.dll", frozenset({"a"}), False, frozenset({"symtab", "unwind"})
        ),
        DebugFile("libfoo.dll", frozenset({"b"}), False, frozenset({"unwind"})),
    ]
    assert library_summary(libraries) == [
        "2 libraries: 1 with function names, 2 with unwind info",
        "No function names: libfoo.dll",
    ]


def test_upload_passes_library_directories(
    monkeypatch: pytest.MonkeyPatch,
) -> None:
    monkeypatch.setenv("SENTRY_AUTH_TOKEN", "t")
    monkeypatch.setenv("SENTRY_PROJECT", "p")
    commands = []

    class Result:
        returncode = 0

    def run(command: list[str], **kwargs: object) -> Result:
        commands.append(command)
        return Result()

    monkeypatch.setattr(sentry_symbols.subprocess, "run", run)
    assert sentry_symbols.main(["upload", "symbols", "app", "dlls"]) == 0
    assert commands == [
        ["sentry-cli", "debug-files", "upload", "symbols", "app", "dlls"]
    ]
