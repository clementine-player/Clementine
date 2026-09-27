"""Browsing Clementine's Internet sidebar (REQUEST_BROWSE) by titles.

Node ids only last as long as the connection, so a path of titles is what
survives from one command to the next.
"""

from __future__ import annotations

import asyncio

from .connection import Connection
from .proto import pb

# How long to wait for a node that is still loading.
DEFAULT_WAIT = 10.0

ACTIONS: dict[str, pb.BrowseAddAction] = {
    "append": pb.BROWSE_ADD_ACTION_APPEND,
    "play-now": pb.BROWSE_ADD_ACTION_PLAY_NOW,
    "play-next": pb.BROWSE_ADD_ACTION_PLAY_NEXT,
    "replace": pb.BROWSE_ADD_ACTION_REPLACE,
}


class BrowseError(Exception):
    pass


async def _next(conn: Connection, msg_type: pb.MsgType, within: float) -> pb.Message:
    async def find() -> pb.Message:
        async for msg in conn.messages():
            if msg.type == msg_type:
                return msg
        raise BrowseError("Connection closed")

    return await asyncio.wait_for(find(), within)


async def browse(
    conn: Connection, node_id: str = "", wait: float = DEFAULT_WAIT
) -> pb.ResponseBrowse:
    """Lists a node's children, waiting while the node is still loading."""
    await conn.send(
        pb.Message(
            type=pb.REQUEST_BROWSE,
            request_browse=pb.RequestBrowse(node_id=node_id),
        )
    )
    loop = asyncio.get_running_loop()
    deadline = loop.time() + wait
    latest: pb.ResponseBrowse | None = None
    while True:
        remaining = deadline - loop.time()
        try:
            # The first answer is always waited for; updates only until the
            # deadline, after which the node is shown as it stands.
            msg = await _next(
                conn, pb.BROWSE, max(0.1, remaining) if latest else DEFAULT_WAIT
            )
        except TimeoutError:
            if latest is None:
                raise BrowseError("Clementine didn't answer") from None
            return latest
        response = msg.response_browse
        if response.node_id != node_id:
            continue
        latest = response
        if response.state != pb.BROWSE_STATE_LOADING or remaining <= 0:
            return response


async def browse_path(
    conn: Connection, titles: list[str], wait: float = DEFAULT_WAIT
) -> tuple[pb.BrowseNode | None, pb.ResponseBrowse]:
    """Walks from the services down |titles|.

    Returns the last node walked into (None for the services) and its
    children.
    """
    node: pb.BrowseNode | None = None
    response = await browse(conn, "", wait)
    for title in titles:
        matches = [n for n in response.nodes if n.title == title]
        if not matches:
            available = ", ".join(n.title for n in response.nodes) or "nothing"
            raise BrowseError(f"No {title!r} here; there's {available}")
        node = matches[0]
        response = await browse(conn, node.node_id, wait)
    return node, response


async def add(
    conn: Connection, node_ids: list[str], action: pb.BrowseAddAction
) -> pb.BrowseAddResult:
    await conn.send(
        pb.Message(
            type=pb.REQUEST_BROWSE_ADD,
            request_browse_add=pb.RequestBrowseAdd(node_ids=node_ids, action=action),
        )
    )
    msg = await _next(conn, pb.BROWSE_ADD_RESULT, DEFAULT_WAIT)
    return msg.response_browse_add.result


def describe(node: pb.BrowseNode) -> str:
    kind = pb.BrowseNodeKind.Name(node.kind).removeprefix("BROWSE_NODE_KIND_").lower()
    line = f"{kind:14} {node.title}"
    if node.subtitle:
        line += f" - {node.subtitle}"
    if node.children == pb.BROWSE_CHILDREN_SOME:
        line += " /"
    if node.playability == pb.BROWSE_PLAYABILITY_ADDABLE:
        line += "  [addable]"
    return line
