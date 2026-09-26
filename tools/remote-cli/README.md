# clementine-remote

A command line client for Clementine's network remote. Its main job is to
exercise remote playback (see `docs/design/remote-streaming.md`): it can act
as a **renderer**, so Clementine plays through it instead of the local sound
card, and it can act as a **controller** that lists outputs and moves playback
between them.

It needs [uv](https://docs.astral.sh/uv/). The protocol's `.proto` file is read
from this repository and compiled, with type stubs, into
`src/clementine_remote/_generated` (not checked in) on first use. It's
recompiled whenever the `.proto` changes.

## Setting up Clementine

Remote playback is a prototype, so it's only there when Clementine is started
with a flag:

```sh
clementine --experimental-remote-streaming
```

Then in *Settings → Network Remote*, turn on *Use a network remote control* and
*Allow playing on remote devices*; the second checkbox is hidden without the
flag. Note the port (5500 by default).

Connect using this machine's LAN address rather than `127.0.0.1`: with *Only
allow connections from the local network* on, Clementine currently refuses
IPv4 localhost when it arrives as `::ffff:127.0.0.1`.

## Examples

```sh
cd tools/remote-cli
alias cr='uv run clementine-remote --host 192.168.1.10'

cr info                  # version, state and features (look for SERVER_FEATURE_RENDERING)
cr outputs               # where playback can go; * marks the active one

# Be a renderer and take over playback. --player null fetches the stream
# without playing it, which is handy on a machine with no audio.
cr render --player ffplay --take-over

# In another terminal: queue something, then move playback around.
cr add ~/Music/song.flac --play
cr use local             # back to Clementine's own output
cr use <renderer id>     # and back to the renderer
cr control pause
cr seek 60
cr watch                 # print state, position and output changes
```

### Renderer options

| Option | Effect |
|---|---|
| `--player null\|ffplay` | `ffplay` plays the audio. `null` only fetches the stream, logging the HTTP status, type, length and ranges it gets. |
| `--format FORMAT` | A format the renderer accepts; repeat for more. It's a MIME type, optionally with `codecs=`, plus two limits of this tool's own: `rates=` (the sample rates it plays) and `channels=` (the most channels). For example `--format "audio/flac; rates=44100,48000; channels=2"`. `--format audio/mpeg` alone makes Clementine encode everything that isn't MP3. |
| `--fail-format FORMAT` | For testing: refuse items in this format as if the player couldn't decode them. Clementine should retry the item once, encoded to another format the renderer accepts, then skip it if that fails too. `--fail-format audio/flac` exercises the retry; failing every format you accept exercises the skip. |
| `--max-bitrate KBPS` | Asks Clementine to keep streams under this bitrate. |
| `--gapless` | Accepts `RENDER_PRELOAD` and starts the next item without a gap. Only the `null` player uses it. |
| `--take-over` | Makes this renderer the active output as soon as it connects. |
| `--id`, `--name` | The renderer's id and display name. The id defaults to a UUID saved in `~/.config/clementine-remote-cli/renderer_id`. |

To try the `ffplay` player without sound, set `SDL_AUDIODRIVER=dummy`.

## Development

All the code is fully typed. CI's Python job runs these, with the versions of
ruff and ty pinned in `pyproject.toml`:

```sh
uv run python -m clementine_remote.proto   # generate the protobuf module first
uv run ruff check
uv run ruff format --check
uv run ty check
uv run pytest
```

Set `CLEMENTINE_REMOTE_PROTO` to use a `.proto` file from elsewhere.

### Smoke tests

`tests/smoke` runs a real Clementine and plays through the renderer, covering
Direct and Pipeline streaming, retries, gapless preloading, moving playback
between outputs and the checks on media URLs. They're skipped unless
`CLEMENTINE_BINARY` points at a built `clementine`, and need `gst-launch-1.0`
with GStreamer's base, good and bad plugins. CI's Test job runs them.

```sh
CLEMENTINE_BINARY=/path/to/build/clementine uv run pytest tests/smoke -v
```

Each test starts Clementine with a throwaway profile and its own `TMPDIR`, so
it can't touch your library or hand off to a Clementine you're running.
