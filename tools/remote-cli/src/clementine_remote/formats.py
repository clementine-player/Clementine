"""Parses --format arguments into AudioFormat messages."""

from __future__ import annotations

import argparse

from .proto import pb

DEFAULT_FORMATS = [
    "audio/mpeg",
    "audio/flac",
    "audio/ogg",
    "audio/aac",
    "audio/mp4",
    "audio/wav",
]


def parse_format(text: str) -> pb.AudioFormat:
    """Parses "audio/flac; rates=44100,48000; channels=2".

    rates= and channels= are this tool's own parameters and are removed from
    the MIME type; others, such as codecs=, stay in it.
    """
    parts = [part.strip() for part in text.split(";")]
    mime_params: list[str] = []
    rates: list[int] = []
    channels = 0

    try:
        for param in parts[1:]:
            name, _, value = param.partition("=")
            name = name.strip().lower()
            if name == "rates":
                rates = [int(rate) for rate in value.split(",") if rate.strip()]
            elif name == "channels":
                channels = int(value)
            else:
                mime_params.append(param)
    except ValueError as e:
        raise argparse.ArgumentTypeError(f"bad number in {text!r}: {e}") from e

    if "/" not in parts[0]:
        raise argparse.ArgumentTypeError(f"{parts[0]!r} isn't a MIME type")

    audio_format = pb.AudioFormat(
        mime_type="; ".join([parts[0], *mime_params]), sample_rates_hz=rates
    )
    if channels:
        audio_format.max_channels = channels
    return audio_format


def describe(audio_format: pb.AudioFormat) -> str:
    text = audio_format.mime_type
    if audio_format.sample_rates_hz:
        text += f" @ {','.join(str(r) for r in audio_format.sample_rates_hz)} Hz"
    if audio_format.max_channels:
        text += f" <= {audio_format.max_channels}ch"
    return text
