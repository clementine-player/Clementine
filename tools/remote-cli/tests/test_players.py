import time

from clementine_remote.players import with_start
from clementine_remote.renderer import Clock


def test_with_start_adds_t():
    assert with_start("http://h:5500/s/tok/3", 1500) == "http://h:5500/s/tok/3?t=1500"


def test_with_start_replaces_t():
    assert (
        with_start("http://h:5500/s/tok/3?t=100", 2000)
        == "http://h:5500/s/tok/3?t=2000"
    )


def test_with_start_zero_removes_t():
    assert with_start("http://h:5500/s/tok/3?t=100", 0) == "http://h:5500/s/tok/3"


def test_clock_runs_only_while_playing():
    clock = Clock()
    clock.set(1000, running=False)
    time.sleep(0.05)
    assert clock.position_ms() == 1000

    clock.resume()
    time.sleep(0.05)
    assert clock.position_ms() >= 1040

    clock.pause()
    paused_at = clock.position_ms()
    time.sleep(0.05)
    assert clock.position_ms() == paused_at
