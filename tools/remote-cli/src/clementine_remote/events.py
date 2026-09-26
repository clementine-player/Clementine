"""What a renderer reports doing, for tests and anything else observing it."""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass, field

from .proto import pb


@dataclass(frozen=True)
class Loaded:
    """Clementine sent RENDER_LOAD."""

    item: pb.RenderItem
    start_ms: int
    playing: bool


@dataclass(frozen=True)
class Preloaded:
    """Clementine sent RENDER_PRELOAD."""

    item: pb.RenderItem


@dataclass(frozen=True)
class Seeked:
    """Clementine sent RENDER_SEEK."""

    position_ms: int
    url: str


@dataclass(frozen=True)
class Stopped:
    """Clementine sent RENDER_STOP."""


@dataclass(frozen=True)
class StateChanged:
    """The renderer's state changed, and it reported the new one."""

    state: pb.RendererState


@dataclass(frozen=True)
class Ended:
    """An item played to its end."""

    item_id: int


@dataclass(frozen=True)
class Failed:
    """The renderer reported an error for an item."""

    item_id: int
    message: str


@dataclass(frozen=True)
class Fetched:
    """The null player finished fetching a URL."""

    url: str
    status: int
    headers: dict[str, str] = field(default_factory=dict)
    # Only kept when the player's keep_bodies is set.
    body: bytes = b""
    size: int = 0


Event = Loaded | Preloaded | Seeked | Stopped | StateChanged | Ended | Failed | Fetched
EventCallback = Callable[[Event], None]


def ignore(event: Event) -> None:
    """An EventCallback that does nothing."""
