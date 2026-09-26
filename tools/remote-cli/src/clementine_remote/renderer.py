"""Acts as a renderer: Clementine's audio output on this machine."""

from __future__ import annotations

import asyncio
import time

from .connection import Connection
from .players import PLAYERS, Player
from .proto import pb

STATUS_INTERVAL = 1.0
# How often to check whether a track has run out, for players that don't
# report it themselves.
TICK = 0.25


class Clock:
    """The renderer's idea of the position within the current item."""

    def __init__(self) -> None:
        self._base_ms = 0
        self._started: float | None = None

    def set(self, position_ms: int, running: bool) -> None:
        self._base_ms = position_ms
        self._started = time.monotonic() if running else None

    def pause(self) -> None:
        self.set(self.position_ms(), False)

    def resume(self) -> None:
        if self._started is None:
            self._started = time.monotonic()

    def position_ms(self) -> int:
        if self._started is None:
            return self._base_ms
        return self._base_ms + int((time.monotonic() - self._started) * 1000)


class Renderer:
    def __init__(self, conn: Connection, player_name: str, gapless: bool, log):
        self.conn = conn
        self.log = log
        self.gapless = gapless
        self.player: Player = PLAYERS[player_name](log, self._ended, self._error)
        self.item: pb.RenderItem | None = None
        self.preloaded: pb.RenderItem | None = None
        self.state = pb.RENDERER_STATE_IDLE
        self.clock = Clock()

    # Status reports ---------------------------------------------------------

    async def send_status(self) -> None:
        position = self.clock.position_ms()
        if self.item and self.item.length_ms:
            position = min(position, self.item.length_ms)
        await self.conn.send(
            pb.RENDERER_STATUS,
            renderer_status=pb.RendererStatus(
                item_id=self.item.item_id if self.item else 0,
                state=self.state,
                position_ms=position,
            ),
        )

    async def set_state(self, state: int) -> None:
        if state != self.state:
            self.state = state
            self.log(f"state {pb.RendererState.Name(state)}")
        await self.send_status()

    async def status_loop(self) -> None:
        while True:
            await asyncio.sleep(STATUS_INTERVAL)
            if self.state == pb.RENDERER_STATE_PLAYING:
                await self.send_status()

    async def end_loop(self) -> None:
        while True:
            await asyncio.sleep(TICK)
            if (
                not self.player.reports_end
                and self.item
                and self.item.length_ms
                and self.state == pb.RENDERER_STATE_PLAYING
                and self.clock.position_ms() >= self.item.length_ms
            ):
                await self._ended()

    # Player callbacks -------------------------------------------------------

    async def _ended(self) -> None:
        if not self.item:
            return
        ended = self.item
        self.log(f"item {ended.item_id} ended")

        if self.preloaded:
            # Start the next item straight away, then tell Clementine; it
            # recognises the item it preloaded and doesn't reload it.
            self.item, self.preloaded = self.preloaded, None
            self.clock.set(0, True)
            await self._start_player(self.item, 0, True)
            await self.conn.send(
                pb.RENDERER_TRACK_ENDED,
                renderer_track_ended=pb.RendererTrackEnded(item_id=ended.item_id),
            )
            await self.send_status()
            return

        self.clock.set(0, False)
        await self.conn.send(
            pb.RENDERER_TRACK_ENDED,
            renderer_track_ended=pb.RendererTrackEnded(item_id=ended.item_id),
        )
        await self.set_state(pb.RENDERER_STATE_IDLE)

    async def _error(self, message: str) -> None:
        self.log(f"error: {message}")
        await self.conn.send(
            pb.RENDERER_ERROR,
            renderer_error=pb.RendererError(
                item_id=self.item.item_id if self.item else 0,
                message=message,
                scope=pb.RENDERER_ERROR_SCOPE_ITEM,
            ),
        )

    # Commands from Clementine ----------------------------------------------

    async def _start_player(
        self, item: pb.RenderItem, start_ms: int, playing: bool
    ) -> None:
        byte_range = item.seek_method == pb.SEEK_METHOD_BYTE_RANGE
        await self.player.load(item.url, start_ms, playing, byte_range, item.length_ms)

    def describe(self, item: pb.RenderItem) -> str:
        song = item.song
        title = " - ".join(x for x in (song.artist, song.title) if x) or item.url
        return (
            f"item {item.item_id}: {title} [{item.mime_type},"
            f" {pb.StreamMode.Name(item.mode)}, {pb.SeekMethod.Name(item.seek_method)},"
            f" {item.length_ms / 1000:.0f}s]"
        )

    async def handle(self, msg: pb.Message) -> None:
        t = msg.type
        if t == pb.RENDER_LOAD:
            load = msg.request_render_load
            playing = load.start_state == pb.LOAD_START_STATE_PLAYING
            self.item, self.preloaded = load.item, None
            self.log(
                f"load {self.describe(load.item)} at {load.start_ms}ms"
                f"{'' if playing else ' (paused)'}"
            )
            self.log(f"  {load.item.url}")
            self.clock.set(load.start_ms, playing)
            await self._start_player(load.item, load.start_ms, playing)
            await self.set_state(
                pb.RENDERER_STATE_PLAYING if playing else pb.RENDERER_STATE_PAUSED
            )

        elif t == pb.RENDER_PRELOAD:
            item = msg.request_render_preload.item
            self.log(f"preload {self.describe(item)}")
            if self.gapless:
                self.preloaded = item

        elif t == pb.RENDER_PLAY:
            if self.item and self.state != pb.RENDERER_STATE_PLAYING:
                self.clock.resume()
                await self.player.resume(self.clock.position_ms())
                await self.set_state(pb.RENDERER_STATE_PLAYING)

        elif t == pb.RENDER_PAUSE:
            if self.state == pb.RENDERER_STATE_PLAYING:
                self.clock.pause()
                await self.player.pause(self.clock.position_ms())
                await self.set_state(pb.RENDERER_STATE_PAUSED)

        elif t == pb.RENDER_STOP:
            self.log("stop")
            await self.player.stop()
            self.item = self.preloaded = None
            self.clock.set(0, False)
            await self.set_state(pb.RENDERER_STATE_IDLE)

        elif t == pb.RENDER_SEEK:
            seek = msg.request_render_seek
            if not self.item or seek.item_id != self.item.item_id:
                return
            self.log(
                f"seek to {seek.position_ms}ms"
                + (f" via {seek.url}" if seek.url else "")
            )
            self.clock.set(seek.position_ms, self.state == pb.RENDERER_STATE_PLAYING)
            await self.player.seek(seek.position_ms, seek.url or None)
            await self.send_status()

        elif t == pb.RENDER_SET_VOLUME:
            volume = msg.request_render_volume.volume
            self.log(f"volume {volume}")
            await self.player.set_volume(volume)

    async def run(self) -> None:
        tasks = [
            asyncio.create_task(self.status_loop()),
            asyncio.create_task(self.end_loop()),
        ]
        try:
            async for msg in self.conn.messages():
                if msg.type == pb.DISCONNECT:
                    reason = pb.ReasonDisconnect.Name(
                        msg.response_disconnect.reason_disconnect
                    )
                    self.log(f"Clementine disconnected: {reason}")
                    return
                if msg.type == pb.OUTPUTS:
                    active = [
                        o.display_name
                        for o in msg.response_outputs.outputs
                        if o.state == pb.OUTPUT_STATE_ACTIVE
                    ]
                    self.log(f"active output: {', '.join(active)}")
                await self.handle(msg)
        finally:
            for task in tasks:
                task.cancel()
            await self.player.stop()
