import argparse

import pytest

from clementine_remote.formats import describe, matches, parse_format


def test_plain_mime_type() -> None:
    fmt = parse_format("audio/mpeg")
    assert fmt.mime_type == "audio/mpeg"
    assert list(fmt.sample_rates_hz) == []
    assert not fmt.HasField("max_channels")


def test_limits_are_removed_from_the_mime_type() -> None:
    fmt = parse_format("audio/ogg; codecs=opus; rates=48000; channels=2")
    assert fmt.mime_type == "audio/ogg; codecs=opus"
    assert list(fmt.sample_rates_hz) == [48000]
    assert fmt.max_channels == 2
    assert describe(fmt) == "audio/ogg; codecs=opus @ 48000 Hz <= 2ch"


def test_several_rates() -> None:
    fmt = parse_format("audio/flac;rates=44100,48000,96000")
    assert list(fmt.sample_rates_hz) == [44100, 48000, 96000]


@pytest.mark.parametrize(
    "text", ["flac", "audio/flac; rates=fast", "audio/x; channels=two"]
)
def test_rejects_bad_input(text: str) -> None:
    with pytest.raises(argparse.ArgumentTypeError):
        parse_format(text)


@pytest.mark.parametrize(
    ("entry", "mime_type", "expected"),
    [
        ("audio/flac", "audio/flac", True),
        ("audio/ogg", "audio/ogg; codecs=opus", True),
        ("audio/ogg; codecs=vorbis", "audio/ogg; codecs=opus", False),
        ("audio/ogg; codecs=opus", "audio/ogg; codecs=opus", True),
        ("audio/flac", "audio/mpeg", False),
    ],
)
def test_matches(entry: str, mime_type: str, expected: bool) -> None:
    assert matches(parse_format(entry), mime_type) is expected
