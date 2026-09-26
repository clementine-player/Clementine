"""Clementine playing through a renderer, end to end."""

from __future__ import annotations

import asyncio
import functools
import socket
from collections.abc import Callable, Coroutine
from pathlib import Path
from typing import Any, ParamSpec

import httpx
import pytest

from clementine_remote.events import Ended, Failed, Loaded, Preloaded, Stopped
from clementine_remote.proto import pb

from .harness import (
    HOST,
    Clementine,
    Controller,
    TestRenderer,
    decoded_seconds,
    has_gst_element,
)

# Encoders can pad or trim a little at either end.
LENGTH_TOLERANCE = 0.35

P = ParamSpec("P")


def run_async(
    test: Callable[P, Coroutine[Any, Any, None]],
) -> Callable[P, None]:
    """Lets pytest run an async test without a plugin."""

    @functools.wraps(test)
    def wrapper(*args: P.args, **kwargs: P.kwargs) -> None:
        asyncio.run(test(*args, **kwargs))

    return wrapper


@run_async
async def test_advertises_rendering(clementine: Clementine) -> None:
    async with Controller(clementine.port) as controller:
        assert pb.SERVER_FEATURE_RENDERING in controller.info.features


@run_async
async def test_nothing_changes_without_the_flag(
    clementine_without_streaming: Clementine, music: dict[str, Path]
) -> None:
    port = clementine_without_streaming.port
    async with Controller(port) as controller:
        # The setting is on, but without the flag it does nothing.
        assert pb.SERVER_FEATURE_RENDERING not in controller.info.features

        # The remote still controls playback on this computer.
        await controller.add(music["long.flac"])
        await controller.wait_for_state(pb.Playing)

        # Output requests are ignored rather than answered.
        await controller.conn.send(pb.Message(type=pb.REQUEST_OUTPUTS))
        await asyncio.sleep(1)
        assert controller.outputs == []

    # Nothing answers HTTP: to the remote protocol it's an oversized message.
    with _connect_from(HOST, port) as http:
        http.sendall(b"GET /s/x/1 HTTP/1.1\r\nHost: x\r\n\r\n")
        assert _read_all(http) == b""


@run_async
async def test_direct_serves_the_file_with_ranges(
    clementine: Clementine, music: dict[str, Path]
) -> None:
    track = music["tone.mp3"]
    async with (
        Controller(clementine.port) as controller,
        TestRenderer(clementine.port) as renderer,
    ):
        await renderer.take_over(controller)
        await controller.add(track)

        item = (await renderer.wait(Loaded)).item
        assert item.mode == pb.STREAM_MODE_DIRECT
        assert item.mime_type == "audio/mpeg"
        assert item.seek_method == pb.SEEK_METHOD_BYTE_RANGE

        response = await renderer.fetched(item.url)
        assert response.status == 200
        assert response.body == track.read_bytes()

        async with httpx.AsyncClient() as client:
            ranged = await client.get(item.url, headers={"Range": "bytes=100-199"})
        assert ranged.status_code == 206
        assert ranged.content == track.read_bytes()[100:200]

        await renderer.wait(Ended, where=lambda e: e.item_id == item.item_id)
        await controller.wait_for_state(pb.Empty)


@run_async
async def test_pipeline_encodes_what_the_renderer_cant_play(
    clementine: Clementine, music: dict[str, Path], tmp_path: Path
) -> None:
    async with (
        Controller(clementine.port) as controller,
        TestRenderer(clementine.port, formats=["audio/mpeg"]) as renderer,
    ):
        await renderer.take_over(controller)
        await controller.add(music["tone.flac"])

        item = (await renderer.wait(Loaded)).item
        assert item.mode == pb.STREAM_MODE_PIPELINE
        assert item.mime_type == "audio/mpeg"
        assert item.seek_method == pb.SEEK_METHOD_NEW_URL

        response = await renderer.fetched(item.url)
        assert response.status == 200
        assert response.headers["transfer-encoding"] == "chunked"
        assert decoded_seconds(response.body, tmp_path) == pytest.approx(
            3.0, abs=LENGTH_TOLERANCE
        )

        # Seeking asks for a new stream that starts part way in.
        async with httpx.AsyncClient() as client:
            later = await client.get(item.url + "?t=1500")
        assert later.status_code == 200
        assert decoded_seconds(later.content, tmp_path) == pytest.approx(
            1.5, abs=LENGTH_TOLERANCE
        )


@run_async
async def test_refused_format_is_retried_encoded(
    clementine: Clementine, music: dict[str, Path], tmp_path: Path
) -> None:
    async with (
        Controller(clementine.port) as controller,
        TestRenderer(clementine.port, fail_formats=["audio/flac"]) as renderer,
    ):
        await renderer.take_over(controller)
        await controller.add(music["tone.flac"])

        first = (await renderer.wait(Loaded)).item
        assert first.mode == pb.STREAM_MODE_DIRECT
        assert first.mime_type == "audio/flac"
        await renderer.wait(Failed, where=lambda e: e.item_id == first.item_id)

        retry = await renderer.wait(Loaded, count=2)
        assert retry.start_ms == 0
        assert retry.item.mode == pb.STREAM_MODE_PIPELINE
        assert retry.item.mime_type != "audio/flac"

        response = await renderer.fetched(retry.item.url)
        assert decoded_seconds(response.body, tmp_path) == pytest.approx(
            3.0, abs=LENGTH_TOLERANCE
        )


@run_async
async def test_track_is_skipped_when_the_retry_fails_too(
    clementine: Clementine, music: dict[str, Path]
) -> None:
    async with (
        Controller(clementine.port) as controller,
        TestRenderer(
            clementine.port, formats=["audio/mpeg"], fail_formats=["audio/mpeg"]
        ) as renderer,
    ):
        await renderer.take_over(controller)
        await controller.add(music["tone.mp3"], music["tone.ogg"])

        # Each track once as it is, once encoded, then Clementine moves on.
        loads = [await renderer.wait(Loaded, count=n) for n in range(1, 5)]
        files = [load.item.song.filename for load in loads]
        assert files[0] == files[1] != files[2] == files[3]
        await controller.wait_for_state(pb.Empty)
        await asyncio.sleep(1)
        assert len(renderer.all(Loaded)) == 4


@run_async
async def test_gapless_preload_is_used_without_a_reload(
    clementine: Clementine, music: dict[str, Path]
) -> None:
    async with (
        Controller(clementine.port) as controller,
        TestRenderer(clementine.port, gapless=True) as renderer,
    ):
        await renderer.take_over(controller)
        await controller.add(music["tone.mp3"], music["tone.ogg"])

        first = (await renderer.wait(Loaded)).item
        second = (await renderer.wait(Preloaded)).item
        await renderer.wait(Ended, where=lambda e: e.item_id == first.item_id)
        await renderer.fetched(second.url)
        await renderer.wait(Ended, where=lambda e: e.item_id == second.item_id)
        assert len(renderer.all(Loaded)) == 1


@pytest.mark.skipif(
    not has_gst_element("fakeaudiosink"),
    reason="needs fakeaudiosink, so local playback keeps time",
)
@run_async
async def test_handing_playback_over_keeps_the_position(
    clementine: Clementine, music: dict[str, Path]
) -> None:
    async with (
        Controller(clementine.port) as controller,
        TestRenderer(clementine.port) as renderer,
    ):
        await renderer.take_over(controller)
        await controller.add(music["long.flac"])
        await renderer.wait(Loaded)
        await asyncio.sleep(2)

        await controller.use("local")
        await renderer.wait(Stopped)
        await asyncio.sleep(2)

        await controller.use(renderer.renderer_id)
        resumed = await renderer.wait(Loaded, count=2)
        # About 4 s in: 2 s on the renderer, then 2 s here.
        assert 3000 <= resumed.start_ms <= 6000
        assert resumed.playing


@run_async
async def test_losing_the_renderer_pauses_here(
    clementine: Clementine, music: dict[str, Path]
) -> None:
    async with Controller(clementine.port) as controller:
        async with TestRenderer(clementine.port) as renderer:
            await renderer.take_over(controller)
            await controller.add(music["long.flac"])
            await renderer.wait(Loaded)
            await controller.wait_for_state(pb.Playing)
            await asyncio.sleep(1)
        # The renderer has disconnected.
        await controller.wait_for_state(pb.Paused)
        assert any(
            o.output_id == "local" and o.state == pb.OUTPUT_STATE_ACTIVE
            for o in controller.outputs
        )


def _connect_from(source: str, port: int) -> socket.socket:
    sock = socket.create_connection((HOST, port), timeout=5, source_address=(source, 0))
    sock.settimeout(5)
    return sock


def _read_all(sock: socket.socket) -> bytes:
    data = b""
    try:
        while chunk := sock.recv(4096):
            data += chunk
    except TimeoutError:
        pass
    return data


@run_async
async def test_media_urls_are_guarded(
    clementine: Clementine, music: dict[str, Path]
) -> None:
    async with (
        Controller(clementine.port) as controller,
        TestRenderer(clementine.port) as renderer,
    ):
        await renderer.take_over(controller)
        await controller.add(music["long.flac"])
        item = (await renderer.wait(Loaded)).item
        # /s/<token>/<item id>
        path = httpx.URL(item.url).path
        token = path.split("/")[2]

        async with httpx.AsyncClient() as client:
            wrong_token = await client.get(item.url.replace(token, "0" * len(token)))
            assert wrong_token.status_code == 404
            assert (await client.post(item.url)).status_code == 405

        # Only the renderer's own address may fetch its URLs. Anywhere in
        # 127.0.0.0/8 is local on Linux, so 127.0.0.2 is a second client.
        try:
            other = _connect_from("127.0.0.2", clementine.port)
        except OSError:
            pytest.skip("can't connect from 127.0.0.2 here")
        with other:
            other.sendall(f"GET {path} HTTP/1.1\r\nHost: x\r\n\r\n".encode())
            assert _read_all(other).startswith(b"HTTP/1.1 403")

        # A connection that's neither the remote protocol nor HTTP is closed
        # without a reply.
        with _connect_from(HOST, clementine.port) as garbage:
            garbage.sendall(b"\x7fhello\r\n\r\n")
            assert _read_all(garbage) == b""
