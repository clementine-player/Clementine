import json
import pathlib

import pytest

import sentry_symbols
from sentry_symbols import (
    DebugFile,
    build_id_path,
    library_summary,
    missing_symbols,
    parse_check,
)


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


def test_parse_check_build_id() -> None:
    output = json.dumps(
        {
            "type": "elf",
            "variants": [{"debug_id": "id1", "code_id": "abcdef", "arch": "x86_64"}],
            "features": "symtab, unwind",
            "is_usable": True,
        }
    )
    assert parse_check("clementine", output).code_ids == frozenset({"abcdef"})


def test_build_id_path() -> None:
    assert build_id_path("776dd5066a82") == pathlib.PurePath(
        ".build-id/77/6dd5066a82.debug"
    )


def fake_check_file(monkeypatch: pytest.MonkeyPatch, code_ids: dict[str, str]) -> None:
    def check_file(path: pathlib.Path) -> DebugFile:
        code_id = code_ids.get(path.name)
        return DebugFile(
            str(path),
            frozenset({"id"}),
            False,
            code_ids=frozenset({code_id}) if code_id else frozenset(),
        )

    monkeypatch.setattr(sentry_symbols, "check_file", check_file)


def test_collect_names_debug_files_after_binaries(
    monkeypatch: pytest.MonkeyPatch, tmp_path: pathlib.Path
) -> None:
    fake_check_file(monkeypatch, {"clementine": "aabbcc", "tagreader": "ddeeff"})
    package = tmp_path / "package"
    (package / "usr/bin").mkdir(parents=True)
    (package / "usr/bin/clementine").write_text("clementine")
    (package / "usr/bin/tagreader").write_text("tagreader")
    debug = package / "usr/lib/debug"
    # As a deb has it: the file itself.
    (debug / ".build-id/aa").mkdir(parents=True)
    (debug / ".build-id/aa/bbcc.debug").write_text("clementine debug")
    # As an RPM has it: a link to the file under its path.
    (debug / "usr/bin").mkdir(parents=True)
    (debug / "usr/bin/tagreader-1.0.debug").write_text("tagreader debug")
    (debug / ".build-id/dd").mkdir()
    (debug / ".build-id/dd/eeff.debug").symlink_to("../../usr/bin/tagreader-1.0.debug")
    symbols = tmp_path / "symbols"

    assert (
        sentry_symbols.main(
            [
                "collect",
                str(debug),
                str(symbols),
                str(package / "usr/bin/clementine"),
                str(package / "usr/bin/tagreader"),
            ]
        )
        == 0
    )
    assert {p.name: p.read_text() for p in symbols.iterdir()} == {
        "clementine": "clementine",
        "clementine.debug": "clementine debug",
        "tagreader": "tagreader",
        "tagreader.debug": "tagreader debug",
    }
    assert not (symbols / "tagreader.debug").is_symlink()


def test_collect_without_debug_file(
    monkeypatch: pytest.MonkeyPatch,
    tmp_path: pathlib.Path,
    capsys: pytest.CaptureFixture[str],
) -> None:
    fake_check_file(monkeypatch, {"clementine": "aabbcc"})
    (tmp_path / "clementine").write_text("clementine")
    assert (
        sentry_symbols.main(
            [
                "collect",
                str(tmp_path / "debug"),
                str(tmp_path / "symbols"),
                str(tmp_path / "clementine"),
            ]
        )
        == 1
    )
    assert "aa/bbcc.debug" in capsys.readouterr().out


def test_collect_without_build_id(
    monkeypatch: pytest.MonkeyPatch,
    tmp_path: pathlib.Path,
    capsys: pytest.CaptureFixture[str],
) -> None:
    fake_check_file(monkeypatch, {})
    (tmp_path / "clementine").write_text("clementine")
    assert (
        sentry_symbols.main(
            [
                "collect",
                str(tmp_path),
                str(tmp_path / "symbols"),
                str(tmp_path / "clementine"),
            ]
        )
        == 1
    )
    assert "has no build ID" in capsys.readouterr().out


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
