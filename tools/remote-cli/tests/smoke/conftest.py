"""End-to-end smoke tests: a real Clementine playing through the CLI's
renderer.

Set CLEMENTINE_BINARY to a built clementine to run them; they're skipped
otherwise. They also need gst-launch-1.0, and GStreamer's base, good and bad
plugins.
"""

from __future__ import annotations

import functools
import os
import shutil
import subprocess
import threading
from collections.abc import Iterator
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
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
    # Long enough that its encoded stream can't all sit in socket buffers.
    "ten-minutes.flac": (60000, "audioconvert ! flacenc"),
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


class _QuietHandler(SimpleHTTPRequestHandler):
    def log_message(self, format: str, *args: object) -> None:
        pass


@pytest.fixture(scope="session")
def music_server(music: dict[str, Path]) -> Iterator[str]:
    """An HTTP server for the test music; yields its base URL."""
    directory = next(iter(music.values())).parent
    handler = functools.partial(_QuietHandler, directory=str(directory))
    server = ThreadingHTTPServer(("127.0.0.1", 0), handler)
    thread = threading.Thread(target=server.serve_forever, daemon=True)
    thread.start()
    yield f"http://127.0.0.1:{server.server_address[1]}"
    server.shutdown()


def _run(root: Path, streaming: bool, extra_config: str = "") -> Iterator[Clementine]:
    instance = Clementine(
        Path(os.environ["CLEMENTINE_BINARY"]), root, streaming, extra_config
    )
    instance.start()
    yield instance
    instance.stop()
    # Shown by pytest only when the test failed.
    print(f"--- Clementine log ({instance.log_path}) ---\n{instance.log_tail()}")


@pytest.fixture
def clementine(tmp_path: Path) -> Iterator[Clementine]:
    """Clementine with --experimental-remote-streaming."""
    yield from _run(tmp_path / "profile", streaming=True)


@pytest.fixture
def clementine_with_radio(tmp_path: Path, music_server: str) -> Iterator[Clementine]:
    """Clementine with a saved radio stream, served by music_server."""
    yield from _run(
        tmp_path / "profile",
        streaming=False,
        extra_config=f"""[SavedRadio]
streams\\1\\url={music_server}/long.flac
streams\\1\\name=Smoke radio
streams\\size=1
""",
    )


@pytest.fixture
def clementine_without_streaming(tmp_path: Path) -> Iterator[Clementine]:
    """Clementine as it runs without the flag, with the setting still on."""
    yield from _run(tmp_path / "profile", streaming=False)
