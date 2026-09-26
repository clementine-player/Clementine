"""Audio backends for the renderer.

NullPlayer fetches the stream without playing it, which exercises
Clementine's HTTP side (content types, ranges, chunked pipeline output) on a
machine with no audio. FfplayPlayer plays through ffplay.
"""

from __future__ import annotations

import asyncio
import shutil
import time
from collections.abc import Awaitable, Callable
from urllib.parse import parse_qsl, urlencode, urlsplit, urlunsplit

import httpx

from .events import EventCallback, Fetched, ignore

Log = Callable[[str], None]
Callback = Callable[[], Awaitable[None]]
ErrorCallback = Callable[[str], Awaitable[None]]


def with_start(url: str, start_ms: int) -> str:
    """Returns |url| with its t= start position replaced."""
    parts = urlsplit(url)
    query = [(k, v) for k, v in parse_qsl(parts.query) if k != "t"]
    if start_ms > 0:
        query.append(("t", str(start_ms)))
    return urlunsplit(parts._replace(query=urlencode(query)))


class Player:
    """What the renderer needs from a backend."""

    # True if the backend notices the end of a track itself. Otherwise the
    # renderer ends tracks when its clock passes their length.
    reports_end = False

    def __init__(
        self,
        log: Log,
        on_end: Callback,
        on_error: ErrorCallback,
        on_event: EventCallback = ignore,
    ) -> None:
        self.log = log
        self.on_end = on_end
        self.on_error = on_error
        self.on_event = on_event
        self.volume = 100

    async def load(
        self, url: str, start_ms: int, playing: bool, byte_range: bool, length_ms: int
    ) -> None: ...

    async def pause(self, position_ms: int) -> None: ...

    async def resume(self, position_ms: int) -> None: ...

    async def seek(self, position_ms: int, url: str | None) -> None: ...

    async def stop(self) -> None: ...

    async def set_volume(self, volume: int) -> None:
        self.volume = volume


class NullPlayer(Player):
    def __init__(
        self,
        log: Log,
        on_end: Callback,
        on_error: ErrorCallback,
        on_event: EventCallback = ignore,
    ) -> None:
        super().__init__(log, on_end, on_error, on_event)
        # Keep each response's body in its Fetched event, for tests that check
        # what Clementine sent.
        self.keep_bodies = False
        self._client = httpx.AsyncClient(timeout=httpx.Timeout(30.0, read=None))
        self._task: asyncio.Task[None] | None = None
        self._url = ""
        self._size: int | None = None
        self._length_ms = 0
        self._byte_range = False

    async def load(
        self, url: str, start_ms: int, playing: bool, byte_range: bool, length_ms: int
    ) -> None:
        self._url = url
        self._byte_range = byte_range
        self._length_ms = length_ms
        self._size = None
        # A Pipeline item's URL already starts at start_ms. A Direct file is
        # fetched from the start, since a time can't be mapped to a byte
        # offset before its size is known.
        self._start_fetch(url, {})

    def _start_fetch(self, url: str, headers: dict[str, str]) -> None:
        if self._task:
            self._task.cancel()
        self._task = asyncio.create_task(self._fetch(url, headers))

    async def _fetch(self, url: str, headers: dict[str, str]) -> None:
        started = time.monotonic()
        received = 0
        body = bytearray()
        try:
            async with self._client.stream("GET", url, headers=headers) as response:
                length = response.headers.get("content-length")
                content_type = response.headers.get("content-type")
                self.log(
                    f"HTTP {response.status_code} {content_type}"
                    f" length={length or 'unknown'}"
                    f" ranges={response.headers.get('accept-ranges')}"
                    + (
                        f" range={response.headers['content-range']}"
                        if "content-range" in response.headers
                        else ""
                    )
                )
                if response.status_code >= 400:
                    self.on_event(
                        Fetched(url, response.status_code, dict(response.headers))
                    )
                    await self.on_error(f"HTTP {response.status_code} for {url}")
                    return
                if length and response.status_code == 200:
                    self._size = int(length)
                async for chunk in response.aiter_raw():
                    received += len(chunk)
                    if self.keep_bodies:
                        body += chunk
            elapsed = time.monotonic() - started
            self.log(f"fetched {received} bytes in {elapsed:.1f}s")
            self.on_event(
                Fetched(
                    url,
                    response.status_code,
                    dict(response.headers),
                    bytes(body),
                    received,
                )
            )
        except asyncio.CancelledError:
            raise
        except httpx.HTTPError as e:
            await self.on_error(f"fetch failed: {e!r}")

    async def pause(self, position_ms: int) -> None:
        pass

    async def resume(self, position_ms: int) -> None:
        pass

    async def seek(self, position_ms: int, url: str | None) -> None:
        if url:
            # Pipeline items: Clementine starts a new pipeline at the position.
            self._start_fetch(url, {})
        elif self._byte_range and self._size and self._length_ms:
            offset = self._size * position_ms // self._length_ms
            self._start_fetch(self._url, {"Range": f"bytes={offset}-"})

    async def stop(self) -> None:
        if self._task:
            self._task.cancel()
            self._task = None


class FfplayPlayer(Player):
    reports_end = True

    def __init__(
        self,
        log: Log,
        on_end: Callback,
        on_error: ErrorCallback,
        on_event: EventCallback = ignore,
    ) -> None:
        super().__init__(log, on_end, on_error, on_event)
        if not shutil.which("ffplay"):
            raise SystemExit("ffplay isn't installed; try --player null")
        self._process: asyncio.subprocess.Process | None = None
        self._watcher: asyncio.Task[None] | None = None
        self._url = ""
        self._byte_range = False

    async def load(
        self, url: str, start_ms: int, playing: bool, byte_range: bool, length_ms: int
    ) -> None:
        await self._kill()
        self._url = url
        self._byte_range = byte_range
        if playing:
            await self._launch(start_ms)

    def _url_at(self, position_ms: int) -> tuple[str, list[str]]:
        # A Direct file can be seeked by ffplay itself. Pipeline output can't:
        # ask Clementine for a stream that starts there instead. This relies
        # on the documented t= parameter, because ffplay has to be restarted
        # to resume after a pause.
        if self._byte_range:
            return self._url, (
                ["-ss", f"{position_ms / 1000:.3f}"] if position_ms else []
            )
        return with_start(self._url, position_ms), []

    async def _launch(self, position_ms: int) -> None:
        url, extra = self._url_at(position_ms)
        args = [
            "ffplay",
            "-nodisp",
            "-autoexit",
            "-nostats",
            "-loglevel",
            "error",
            "-volume",
            str(self.volume),
            *extra,
            url,
        ]
        self.log("$ " + " ".join(args))
        self._process = await asyncio.create_subprocess_exec(
            *args, stdin=asyncio.subprocess.DEVNULL
        )
        self._watcher = asyncio.create_task(self._watch(self._process))

    async def _watch(self, process: asyncio.subprocess.Process) -> None:
        code = await process.wait()
        if process is not self._process:
            return  # We killed it on purpose.
        self._process = None
        if code == 0:
            await self.on_end()
        else:
            await self.on_error(f"ffplay exited with {code}")

    async def _kill(self) -> None:
        process, self._process = self._process, None
        if process and process.returncode is None:
            process.terminate()
            await process.wait()

    async def pause(self, position_ms: int) -> None:
        await self._kill()

    async def resume(self, position_ms: int) -> None:
        await self._launch(position_ms)

    async def seek(self, position_ms: int, url: str | None) -> None:
        # For Pipeline items |url| is the item's URL with t=position_ms, which
        # _launch builds the same way.
        playing = self._process is not None
        await self._kill()
        if playing:
            await self._launch(position_ms)

    async def stop(self) -> None:
        await self._kill()

    async def set_volume(self, volume: int) -> None:
        await super().set_volume(volume)
        if self._process:
            self.log("ffplay can't change volume while playing; applies next track")


PLAYERS: dict[str, type[Player]] = {"null": NullPlayer, "ffplay": FfplayPlayer}
