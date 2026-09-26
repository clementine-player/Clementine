"""End-to-end smoke tests: a real Clementine playing through the CLI's
renderer.

Set CLEMENTINE_BINARY to a built clementine to run them; they're skipped
otherwise. They also need gst-launch-1.0, and GStreamer's base, good and bad
plugins.
"""

from __future__ import annotations

import os
import shutil
import subprocess
from collections.abc import Iterator
from pathlib import Path

import pytest

from .harness import Clementine

# samplesperbuffer=441 at 44.1 kHz makes each buffer 10 ms.
TRACKS: dict[str, tuple[int, str]] = {
    "tone.mp3": (
        300,
        (
            "audioconvert ! lamemp3enc target=bitrate cbr=true"
            " bitrate=128 ! xingmux ! id3v2mux"
        ),
    ),
    "tone.flac": (300, "audioconvert ! flacenc"),
    "tone.ogg": (300, "audioconvert ! vorbisenc ! oggmux"),
    "long.flac": (1500, "audioconvert ! flacenc"),
}


def pytest_collection_modifyitems(
    config: pytest.Config, items: list[pytest.Item]
) -> None:
    reason = None
    if not os.environ.get("CLEMENTINE_BINARY"):
        reason = "set CLEMENTINE_BINARY to a built clementine"
    elif not shutil.which("gst-launch-1.0"):
        reason = "needs gst-launch-1.0"
    if reason:
        for item in items:
            if "smoke" in str(item.path):
                item.add_marker(pytest.mark.skip(reason=reason))


@pytest.fixture(scope="session")
def music(tmp_path_factory: pytest.TempPathFactory) -> dict[str, Path]:
    directory = tmp_path_factory.mktemp("music")
    tracks = {}
    for name, (buffers, encode) in TRACKS.items():
        path = directory / name
        subprocess.run(
            f"gst-launch-1.0 -q audiotestsrc num-buffers={buffers} samplesperbuffer=441"
            f" ! {encode} ! filesink location={path}",
            shell=True,
            check=True,
        )
        tracks[name] = path
    return tracks


@pytest.fixture
def clementine(tmp_path: Path) -> Iterator[Clementine]:
    instance = Clementine(Path(os.environ["CLEMENTINE_BINARY"]), tmp_path / "profile")
    instance.start()
    yield instance
    instance.stop()
    # Shown by pytest only when the test failed.
    print(f"--- Clementine log ({instance.log_path}) ---\n{instance.log_tail()}")
