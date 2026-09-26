# clementine-remote

A command line client for Clementine's network remote. Its main job is to
exercise remote playback (see `docs/design/remote-streaming.md`): it can act
as a **renderer**, so Clementine plays through it instead of the local sound
card, and it can act as a **controller** that lists outputs and moves playback
between them.

It needs [uv](https://docs.astral.sh/uv/). The protocol's `.proto` file is read
from this repository and compiled on first use, so changes to it are picked up
on the next run.

## Setting up Clementine

In *Settings → Network Remote*, turn on *Use a network remote control* and
*Allow playing on remote devices*. Note the port (5500 by default).

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

```sh
uv run --group dev pytest
uvx ruff check src tests && uvx ruff format --check src tests
```

Set `CLEMENTINE_REMOTE_PROTO` to use a `.proto` file from elsewhere.
