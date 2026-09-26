"""The remote's wire format: a 4-byte big-endian length, then a Message."""

from __future__ import annotations

import asyncio
import struct
from collections.abc import AsyncIterator

from .proto import pb

# RemoteClient drops anything longer; see ProtocolSniffer.
MAX_MESSAGE = 128 * 1024 * 1024


class Connection:
    def __init__(self, reader: asyncio.StreamReader, writer: asyncio.StreamWriter):
        self._reader = reader
        self._writer = writer

    @classmethod
    async def open(cls, host: str, port: int) -> Connection:
        reader, writer = await asyncio.open_connection(host, port)
        return cls(reader, writer)

    async def send(self, msg_type: int, **fields) -> None:
        """Sends a Message of |msg_type|; keyword arguments fill its fields."""
        msg = pb.Message(type=msg_type, **fields)
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
        try:
            await self.send(pb.DISCONNECT)
        except (ConnectionError, OSError):
            pass
        self._writer.close()
        try:
            await self._writer.wait_closed()
        except (ConnectionError, OSError):
            pass


async def connect(
    host: str,
    port: int,
    auth_code: int | None = None,
    renderer: pb.RendererCapabilities | None = None,
    send_playlist_songs: bool = False,
) -> tuple[Connection, pb.Message]:
    """Connects and authenticates. Returns the connection and the INFO reply."""
    conn = await Connection.open(host, port)
    request = pb.RequestConnect(send_playlist_songs=send_playlist_songs)
    if auth_code is not None:
        request.auth_code = auth_code
    if renderer is not None:
        request.renderer.CopyFrom(renderer)
    await conn.send(pb.CONNECT, request_connect=request)

    async for msg in conn.messages():
        if msg.type == pb.INFO:
            return conn, msg
        if msg.type == pb.DISCONNECT:
            reason = pb.ReasonDisconnect.Name(msg.response_disconnect.reason_disconnect)
            raise ConnectionError(f"Clementine disconnected us: {reason}")
    raise ConnectionError("Connection closed before Clementine replied")
