"""clementine-remote: talk to Clementine's network remote from a terminal."""

from __future__ import annotations

import argparse
import asyncio
import datetime
import os
import socket
import sys
import uuid
from collections.abc import Callable, Coroutine
from pathlib import Path
from typing import Any

from . import connection
from .connection import Connection
from .formats import DEFAULT_FORMATS, describe, parse_format
from .proto import pb
from .renderer import Renderer


class Args(argparse.Namespace):
    """The parsed command line, with the type of every option."""

    host: str
    port: int
    auth_code: int | None
    func: Callable[[Args], Coroutine[Any, Any, None]]
    # use
    output_id: str
    # control
    action: str
    # seek
    seconds: int
    # add
    items: list[str]
    play: bool
    # render
    name: str
    id: str | None
    player: str
    format: list[pb.AudioFormat] | None
    fail_format: list[pb.AudioFormat] | None
    max_bitrate: int | None
    gapless: bool
    take_over: bool


CONTROLS: dict[str, pb.MsgType] = {
    "play": pb.PLAY,
    "pause": pb.PAUSE,
    "playpause": pb.PLAYPAUSE,
    "stop": pb.STOP,
    "next": pb.NEXT,
    "previous": pb.PREVIOUS,
}


def log(message: str) -> None:
    now = datetime.datetime.now().astimezone().strftime("%H:%M:%S.%f")[:-3]
    print(f"{now} {message}", flush=True)


def renderer_id(explicit: str | None) -> str:
    """A stable id per install, so Clementine recognises us on reconnect."""
    if explicit:
        return explicit
    base = os.environ.get("XDG_CONFIG_HOME") or Path.home() / ".config"
    path = Path(base) / "clementine-remote-cli" / "renderer_id"
    if path.exists():
        return path.read_text().strip()
    path.parent.mkdir(parents=True, exist_ok=True)
    value = str(uuid.uuid4())
    path.write_text(value + "\n")
    return value


async def open_connection(
    args: Args, renderer: pb.RendererCapabilities | None = None
) -> tuple[Connection, pb.ResponseClementineInfo]:
    try:
        conn, info = await connection.connect(
            args.host, args.port, args.auth_code, renderer=renderer
        )
    except OSError as e:
        raise SystemExit(f"Can't connect to {args.host}:{args.port}: {e}") from e
    except ConnectionError as e:
        raise SystemExit(str(e)) from e
    return conn, info


def has_rendering(info: pb.ResponseClementineInfo) -> bool:
    return pb.SERVER_FEATURE_RENDERING in info.features


# How long to wait for Clementine to answer a request.
REPLY_TIMEOUT = 5.0


async def wait_for(conn: Connection, msg_type: pb.MsgType) -> pb.Message:
    async def find() -> pb.Message:
        async for msg in conn.messages():
            if msg.type == msg_type:
                return msg
        raise SystemExit("Connection closed")

    try:
        return await asyncio.wait_for(find(), REPLY_TIMEOUT)
    except TimeoutError as e:
        raise SystemExit(f"No {pb.MsgType.Name(msg_type)} from Clementine") from e


def print_outputs(outputs: pb.ResponseOutputs) -> None:
    for output in outputs.outputs:
        marker = "*" if output.state == pb.OUTPUT_STATE_ACTIVE else " "
        print(f"{marker} {output.output_id:38} {output.display_name}")


# Commands ------------------------------------------------------------------


async def cmd_info(args: Args) -> None:
    conn, info = await open_connection(args)
    print(f"version:  {info.version}")
    print(f"state:    {pb.EngineState.Name(info.state)}")
    print(f"downloads allowed: {info.allow_downloads}")
    features = ", ".join(pb.ServerFeature.Name(f) for f in info.features)
    print(f"features: {features or 'none'}")
    await conn.close()


async def cmd_outputs(args: Args) -> None:
    conn, info = await open_connection(args)
    if not has_rendering(info):
        raise SystemExit("Clementine doesn't allow playing on remote devices")
    await conn.send(pb.Message(type=pb.REQUEST_OUTPUTS))
    msg = await wait_for(conn, pb.OUTPUTS)
    print_outputs(msg.response_outputs)
    await conn.close()


async def cmd_use(args: Args) -> None:
    conn, info = await open_connection(args)
    if not has_rendering(info):
        raise SystemExit("Clementine doesn't allow playing on remote devices")
    await conn.send(
        pb.Message(
            type=pb.SET_OUTPUT,
            request_set_output=pb.RequestSetOutput(output_id=args.output_id),
        )
    )
    msg = await wait_for(conn, pb.OUTPUTS)
    print_outputs(msg.response_outputs)
    await conn.close()


async def cmd_control(args: Args) -> None:
    conn, _ = await open_connection(args)
    await conn.send(pb.Message(type=CONTROLS[args.action]))
    await conn.close()


async def cmd_seek(args: Args) -> None:
    conn, _ = await open_connection(args)
    await conn.send(
        pb.Message(
            type=pb.SET_TRACK_POSITION,
            request_set_track_position=pb.RequestSetTrackPosition(
                position=args.seconds
            ),
        )
    )
    await conn.close()


def to_urls(items: list[str]) -> list[str]:
    """Turns the paths among |items| into file:// URLs."""
    urls: list[str] = []
    for item in items:
        path = Path(item)
        urls.append(path.resolve().as_uri() if path.exists() else item)
    return urls


async def cmd_add(args: Args) -> None:
    conn, _ = await open_connection(args)
    await conn.send(pb.Message(type=pb.REQUEST_PLAYLISTS))
    msg = await wait_for(conn, pb.PLAYLISTS)
    active = [p.id for p in msg.response_playlists.playlist if p.active]
    if not active:
        raise SystemExit("Clementine has no active playlist")

    urls = to_urls(args.items)
    await conn.send(
        pb.Message(
            type=pb.INSERT_URLS,
            request_insert_urls=pb.RequestInsertUrls(
                playlist_id=active[0], urls=urls, play_now=args.play
            ),
        )
    )
    print(f"Added {len(urls)} item(s) to playlist {active[0]}")
    await conn.close()


async def cmd_watch(args: Args) -> None:
    conn, info = await open_connection(args)
    log(f"connected to {info.version}")
    if has_rendering(info):
        await conn.send(pb.Message(type=pb.REQUEST_OUTPUTS))
    async for msg in conn.messages():
        t = msg.type
        if t == pb.CURRENT_METAINFO:
            song = msg.response_current_metadata.song_metadata
            log(f"now playing: {song.artist} - {song.title}")
        elif t in (pb.PLAY, pb.PAUSE, pb.STOP):
            # Clementine reports state changes with these, not
            # ENGINE_STATE_CHANGED.
            log(f"state: {pb.MsgType.Name(t).lower()}")
        elif t == pb.UPDATE_TRACK_POSITION:
            log(f"position: {msg.response_update_track_position.position}s")
        elif t == pb.OUTPUTS:
            active = [
                o.display_name
                for o in msg.response_outputs.outputs
                if o.state == pb.OUTPUT_STATE_ACTIVE
            ]
            log(
                f"outputs: {len(msg.response_outputs.outputs)},"
                f" active: {', '.join(active)}"
            )
        elif t == pb.DISCONNECT:
            log("disconnected")
            return


async def cmd_render(args: Args) -> None:
    caps = pb.RendererCapabilities(
        renderer_id=renderer_id(args.id),
        display_name=args.name,
        formats=args.format or [parse_format(f) for f in DEFAULT_FORMATS],
        features=[pb.RENDERER_FEATURE_HTTP_RANGE]
        + ([pb.RENDERER_FEATURE_GAPLESS] if args.gapless else []),
    )
    if args.max_bitrate:
        caps.max_bitrate_kbps = args.max_bitrate

    conn, info = await open_connection(args, renderer=caps)
    if not has_rendering(info):
        raise SystemExit(
            "Clementine doesn't allow playing on remote devices; turn on"
            " 'Allow playing on remote devices' in the network remote settings"
        )

    log(
        f"registered as '{caps.display_name}' ({caps.renderer_id})"
        f" accepting {'; '.join(describe(f) for f in caps.formats)}"
    )

    renderer = Renderer(conn, args.player, args.gapless, log, args.fail_format)
    if args.take_over:
        await conn.send(
            pb.Message(
                type=pb.SET_OUTPUT,
                request_set_output=pb.RequestSetOutput(output_id=caps.renderer_id),
            )
        )
    await renderer.run()


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="clementine-remote", description=__doc__)
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=5500)
    parser.add_argument("--auth-code", type=int, help="if Clementine asks for one")
    sub = parser.add_subparsers(dest="command", required=True)

    sub.add_parser(
        "info", help="show Clementine's version, state and features"
    ).set_defaults(func=cmd_info)
    sub.add_parser("outputs", help="list where playback can go").set_defaults(
        func=cmd_outputs
    )

    p = sub.add_parser(
        "use", help="move playback to an output ('local' for Clementine)"
    )
    p.add_argument("output_id")
    p.set_defaults(func=cmd_use)

    p = sub.add_parser("control", help="send a playback command")
    p.add_argument("action", choices=sorted(CONTROLS))
    p.set_defaults(func=cmd_control)

    p = sub.add_parser("seek", help="seek the current track")
    p.add_argument("seconds", type=int)
    p.set_defaults(func=cmd_seek)

    p = sub.add_parser("add", help="add files or URLs to the active playlist")
    p.add_argument("items", nargs="+")
    p.add_argument("--play", action="store_true", help="start playing the first one")
    p.set_defaults(func=cmd_add)

    sub.add_parser("watch", help="print what Clementine reports").set_defaults(
        func=cmd_watch
    )

    p = sub.add_parser("render", help="act as a renderer: Clementine plays through us")
    p.add_argument("--name", default=f"clementine-remote on {socket.gethostname()}")
    p.add_argument("--id", help="renderer id (default: one saved per install)")
    p.add_argument(
        "--player",
        choices=["null", "ffplay"],
        default="ffplay",
        help="null fetches the stream without playing it",
    )
    p.add_argument(
        "--format",
        action="append",
        type=parse_format,
        metavar="FORMAT",
        help="a format we can play, as a MIME type with optional rates= and"
        ' channels= limits, e.g. "audio/flac; rates=44100,48000; channels=2".'
        f" Repeat for more (default: {', '.join(DEFAULT_FORMATS)})",
    )
    p.add_argument(
        "--fail-format",
        action="append",
        type=parse_format,
        metavar="FORMAT",
        help="report an error instead of playing items in this format, as if"
        " the player couldn't decode them; Clementine should retry them"
        " encoded to another format. For testing. Repeat for more",
    )
    p.add_argument("--max-bitrate", type=int, help="in kbps")
    p.add_argument(
        "--gapless",
        action="store_true",
        help="accept preloads and start them without a gap",
    )
    p.add_argument(
        "--take-over",
        action="store_true",
        help="make this renderer the active output straight away",
    )
    p.set_defaults(func=cmd_render)
    return parser


def main(argv: list[str] | None = None) -> None:
    args = build_parser().parse_args(argv, namespace=Args())
    try:
        asyncio.run(args.func(args))
    except KeyboardInterrupt:
        pass
    except BrokenPipeError:
        sys.exit(1)
