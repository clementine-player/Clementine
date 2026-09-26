"""The remote's wire format: a 4-byte big-endian length, then a Message."""

from __future__ import annotations

import asyncio
import contextlib
import struct
from collections.abc import AsyncIterator

from .proto import pb

# RemoteClient drops anything longer; see ProtocolSniffer.
MAX_MESSAGE = 128 * 1024 * 1024


class Connection:
    def __init__(
        self, reader: asyncio.StreamReader, writer: asyncio.StreamWriter
    ) -> None:
        self._reader = reader
        self._writer = writer

    @classmethod
    async def open(
        cls, host: str, port: int, local_address: str | None = None
    ) -> Connection:
        """Connects, from |local_address| if it's given."""
        reader, writer = await asyncio.open_connection(
            host, port, local_addr=(local_address, 0) if local_address else None
        )
        return cls(reader, writer)

    async def send(self, msg: pb.Message) -> None:
        data = msg.SerializeToString()
        self._writer.write(struct.pack(">I", len(data)) + data)
        await self._writer.drain()

    async def receive(self) -> pb.Message:
        header = await self._reader.readexactly(4)
        (length,) = struct.unpack(">I", header)
        if length > MAX_MESSAGE:
            raise ConnectionError(f"Message of {length} bytes is too long")
        msg = pb.Message()
        msg.ParseFromString(await self._reader.readexactly(length))
        return msg

    async def messages(self) -> AsyncIterator[pb.Message]:
        try:
            while True:
                yield await self.receive()
        except asyncio.IncompleteReadError:
            return

    async def close(self) -> None:
        with contextlib.suppress(ConnectionError, OSError):
            await self.send(pb.Message(type=pb.DISCONNECT))
        self._writer.close()
        with contextlib.suppress(ConnectionError, OSError):
            await self._writer.wait_closed()


async def connect(
    host: str,
    port: int,
    auth_code: int | None = None,
    renderer: pb.RendererCapabilities | None = None,
    send_playlist_songs: bool = False,
    local_address: str | None = None,
) -> tuple[Connection, pb.ResponseClementineInfo]:
    """Connects and authenticates. Returns the connection and Clementine's INFO."""
    conn = await Connection.open(host, port, local_address)
    request = pb.RequestConnect(send_playlist_songs=send_playlist_songs)
    if auth_code is not None:
        request.auth_code = auth_code
    if renderer is not None:
        request.renderer.CopyFrom(renderer)
    await conn.send(pb.Message(type=pb.CONNECT, request_connect=request))

    async for msg in conn.messages():
        if msg.type == pb.INFO:
            return conn, msg.response_clementine_info
        if msg.type == pb.DISCONNECT:
            reason = pb.ReasonDisconnect.Name(msg.response_disconnect.reason_disconnect)
            raise ConnectionError(f"Clementine disconnected us: {reason}")
    raise ConnectionError("Connection closed before Clementine replied")
