"""Runs a real Clementine and talks to it as a controller and a renderer."""

from __future__ import annotations

import asyncio
import os
import shutil
import socket
import subprocess
import tempfile
import time
from collections.abc import Callable
from pathlib import Path
from types import TracebackType
from typing import Self, TypeVar

from clementine_remote import connection
from clementine_remote.connection import Connection
from clementine_remote.events import Event, Fetched
from clementine_remote.formats import DEFAULT_FORMATS, parse_format
from clementine_remote.players import NullPlayer
from clementine_remote.proto import pb
from clementine_remote.renderer import Renderer

T = TypeVar("T")
E = TypeVar("E", bound=Event)

HOST = "127.0.0.1"
TIMEOUT = 20.0


def free_port() -> int:
    with socket.socket() as s:
        s.bind((HOST, 0))
        return s.getsockname()[1]


def has_gst_element(name: str) -> bool:
    return (
        subprocess.run(
            ["gst-inspect-1.0", "--exists", name], capture_output=True, check=False
        ).returncode
        == 0
    )


class Clementine:
    """A Clementine with a throwaway profile, listening on 127.0.0.1 only."""

    def __init__(self, binary: Path, root: Path) -> None:
        self.binary = binary
        self.root = root
        self.port = free_port()
        self.log_path = root / "clementine.log"
        self.process: subprocess.Popen[bytes] | None = None
        # Short, because Qt's local sockets live here and Unix socket paths
        # are limited to about 100 characters. Also keeps Clementine's
        # single-instance check away from a Clementine the user is running.
        self.tmpdir = Path(tempfile.mkdtemp(prefix="cst-"))

    def write_config(self) -> None:
        config = self.root / ".config" / "Clementine"
        config.mkdir(parents=True, exist_ok=True)
        # A sink that keeps time, so the local engine plays in real time.
        sink = "fakeaudiosink" if has_gst_element("fakeaudiosink") else "fakesink"
        (config / "Clementine.conf").write_text(
            f"""[NetworkRemote]
use_remote=true
port={self.port}
allow_streaming=true
only_non_public_ip=true
listen_on_all_addresses=false
listen_addresses={HOST}
use_auth_code=false

[GstEngine]
sink={sink}
"""
        )

    def start(self) -> None:
        self.write_config()
        env = dict(
            os.environ,
            HOME=str(self.root),
            XDG_CONFIG_HOME=str(self.root / ".config"),
            XDG_DATA_HOME=str(self.root / ".local" / "share"),
            XDG_CACHE_HOME=str(self.root / ".cache"),
            TMPDIR=str(self.tmpdir),
            QT_QPA_PLATFORM="offscreen",
            DBUS_SESSION_BUS_ADDRESS="disabled:",
        )
        self.process = subprocess.Popen(
            [str(self.binary), "--log-levels", "*:3"],
            env=env,
            stdout=self.log_path.open("wb"),
            stderr=subprocess.STDOUT,
            start_new_session=True,
        )
        deadline = time.monotonic() + 60
        while time.monotonic() < deadline:
            code = self.process.poll()
            if code is not None:
                raise RuntimeError(f"Clementine exited with {code}:\n{self.log_tail()}")
            try:
                socket.create_connection((HOST, self.port), timeout=1).close()
                return
            except OSError:
                time.sleep(0.25)
        raise RuntimeError(f"Clementine didn't start listening:\n{self.log_tail()}")

    def stop(self) -> None:
        if self.process and self.process.poll() is None:
            self.process.terminate()
            try:
                self.process.wait(15)
            except subprocess.TimeoutExpired:
                self.process.kill()
                self.process.wait()
        shutil.rmtree(self.tmpdir, ignore_errors=True)

    def log_tail(self, lines: int = 60) -> str:
        try:
            text = self.log_path.read_text(errors="replace").splitlines()
        except FileNotFoundError:
            return "(no log)"
        text = [line for line in text if "TagReader worker" not in line]
        return "\n".join(text[-lines:])


async def eventually(
    predicate: Callable[[], T], within: float = TIMEOUT, what: str = "condition"
) -> T:
    """Waits until predicate() returns something truthy, and returns it."""
    deadline = time.monotonic() + within
    while True:
        result = predicate()
        if result:
            return result
        if time.monotonic() > deadline:
            raise AssertionError(f"Timed out waiting for {what}")
        await asyncio.sleep(0.05)


# Clementine reports engine state changes with these message types.
STATE_MESSAGES: dict[pb.MsgType, pb.EngineState] = {
    pb.PLAY: pb.Playing,
    pb.PAUSE: pb.Paused,
    pb.STOP: pb.Empty,
}


class Controller:
    """A plain remote control that tracks what Clementine reports."""

    def __init__(self, port: int) -> None:
        self.port = port
        # States Clementine has reported since we connected, oldest first.
        self.states: list[pb.EngineState] = []
        self.info = pb.ResponseClementineInfo()
        self.outputs: list[pb.Output] = []
        self.playlists: list[pb.Playlist] = []
        self._conn: Connection | None = None
        self._task: asyncio.Task[None] | None = None

    @property
    def conn(self) -> Connection:
        assert self._conn, "use Controller with async with"
        return self._conn

    async def __aenter__(self) -> Self:
        self._conn, self.info = await connection.connect(HOST, self.port)
        self._task = asyncio.create_task(self._read())
        return self

    async def __aexit__(
        self,
        exc_type: type[BaseException] | None,
        exc: BaseException | None,
        traceback: TracebackType | None,
    ) -> None:
        if self._task:
            self._task.cancel()
        await self.conn.close()

    async def _read(self) -> None:
        async for msg in self.conn.messages():
            if msg.type in STATE_MESSAGES:
                self.states.append(STATE_MESSAGES[msg.type])
            elif msg.type == pb.OUTPUTS:
                self.outputs = list(msg.response_outputs.outputs)
            elif msg.type == pb.PLAYLISTS:
                self.playlists = list(msg.response_playlists.playlist)

    async def add(self, *paths: Path, play: bool = True) -> None:
        self.playlists = []
        await self.conn.send(pb.Message(type=pb.REQUEST_PLAYLISTS))
        playlists = await eventually(lambda: self.playlists, what="playlists")
        active = next(p.id for p in playlists if p.active)
        await self.conn.send(
            pb.Message(
                type=pb.INSERT_URLS,
                request_insert_urls=pb.RequestInsertUrls(
                    playlist_id=active,
                    urls=[p.as_uri() for p in paths],
                    play_now=play,
                ),
            )
        )

    async def use(self, output_id: str) -> None:
        await self.conn.send(
            pb.Message(
                type=pb.SET_OUTPUT,
                request_set_output=pb.RequestSetOutput(output_id=output_id),
            )
        )
        await eventually(
            lambda: any(
                o.output_id == output_id and o.state == pb.OUTPUT_STATE_ACTIVE
                for o in self.outputs
            ),
            what=f"output {output_id} to become active",
        )

    async def wait_for_state(
        self, state: pb.EngineState, within: float = TIMEOUT
    ) -> None:
        """Waits until Clementine reports |state| as its latest."""
        await eventually(
            lambda: bool(self.states) and self.states[-1] == state,
            within,
            f"Clementine to be {pb.EngineState.Name(state)}",
        )


class TestRenderer:
    """The CLI's renderer with its null player, recording every event."""

    __test__ = False  # Not a pytest test class.

    def __init__(
        self,
        port: int,
        renderer_id: str = "smoke-renderer",
        formats: list[str] | None = None,
        fail_formats: list[str] | None = None,
        gapless: bool = False,
    ) -> None:
        self.port = port
        self.renderer_id = renderer_id
        self.formats = formats or DEFAULT_FORMATS
        self.fail_formats = fail_formats or []
        self.gapless = gapless
        self.events: list[Event] = []
        self.log_lines: list[str] = []
        self._conn: Connection | None = None
        self._task: asyncio.Task[None] | None = None

    def _record(self, event: Event) -> None:
        self.events.append(event)

    def _log(self, line: str) -> None:
        self.log_lines.append(line)

    async def __aenter__(self) -> Self:
        features = [pb.RENDERER_FEATURE_HTTP_RANGE]
        if self.gapless:
            features.append(pb.RENDERER_FEATURE_GAPLESS)
        caps = pb.RendererCapabilities(
            renderer_id=self.renderer_id,
            display_name=self.renderer_id,
            formats=[parse_format(f) for f in self.formats],
            features=features,
        )
        self._conn, info = await connection.connect(HOST, self.port, renderer=caps)
        assert pb.SERVER_FEATURE_RENDERING in info.features
        self.renderer = Renderer(
            self._conn,
            "null",
            self.gapless,
            self._log,
            [parse_format(f) for f in self.fail_formats],
            on_event=self._record,
        )
        assert isinstance(self.renderer.player, NullPlayer)
        self.renderer.player.keep_bodies = True
        self._task = asyncio.create_task(self.renderer.run())
        return self

    async def __aexit__(
        self,
        exc_type: type[BaseException] | None,
        exc: BaseException | None,
        traceback: TracebackType | None,
    ) -> None:
        await self.disconnect()

    async def disconnect(self) -> None:
        if self._task and not self._task.done():
            self._task.cancel()
        if self._conn:
            await self._conn.close()
            self._conn = None

    async def take_over(self, controller: Controller) -> None:
        # Registration reaches Clementine's main thread asynchronously; wait
        # until it's listed before asking for it.
        await eventually(
            lambda: any(o.output_id == self.renderer_id for o in controller.outputs),
            what=f"renderer {self.renderer_id} to be listed",
        )
        await controller.use(self.renderer_id)

    def all(
        self, kind: type[E], where: Callable[[E], bool] = lambda event: True
    ) -> list[E]:
        """The events of type |kind| so far, oldest first, that match |where|."""
        return [e for e in self.events if isinstance(e, kind) and where(e)]

    async def wait(
        self,
        kind: type[E],
        count: int = 1,
        where: Callable[[E], bool] = lambda event: True,
        within: float = TIMEOUT,
    ) -> E:
        """Waits for the |count|th event of type |kind| matching |where|."""
        found = await eventually(
            lambda: self.all(kind, where)[count - 1 :],
            within,
            f"renderer event {kind.__name__} #{count}",
        )
        return found[0]

    async def fetched(self, url: str) -> Fetched:
        """The response the null player got for |url|."""
        return await self.wait(Fetched, where=lambda e: e.url == url)


def decoded_seconds(data: bytes, work_dir: Path) -> float:
    """Decodes |data| with GStreamer and returns how much audio it holds."""
    source = work_dir / "decode-in"
    raw = work_dir / "decode-out.raw"
    source.write_bytes(data)
    subprocess.run(
        [
            "gst-launch-1.0",
            "-q",
            "filesrc",
            f"location={source}",
            "!",
            "decodebin",
            "!",
            "audioconvert",
            "!",
            "audioresample",
            "!",
            "audio/x-raw,format=S16LE,channels=1,rate=8000",
            "!",
            "filesink",
            f"location={raw}",
        ],
        check=True,
        capture_output=True,
        timeout=60,
    )
    return raw.stat().st_size / (2 * 8000)
