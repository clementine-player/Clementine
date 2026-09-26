# Design: streaming to remote devices through the network remote

Status: proposal, with a prototype (see §14)
Scope: `src/networkremote`, `src/engines`, `src/core/player`,
`ext/libclementine-remote/remotecontrolmessages.proto`

## 1. Summary

Today the network remote lets a device *control* Clementine: Clementine
decodes and plays the audio on the computer it runs on. A device can also
*download* files (`DOWNLOAD_SONGS`), which copies them rather than playing
them.

This design adds a third role: **the remote device becomes the speaker.**
Clementine keeps everything it does today (the library, playlists, queue,
shuffle and repeat, internet services, scrobbling, now-playing, MPRIS) but
sends the audio to a device on the network instead of its local sound card.

It covers two cases:

- **Streaming**: the device plays the original file, which Clementine serves
  over HTTP. This covers most of a local library.
- **Restreaming**: Clementine fetches the source through a GStreamer
  pipeline and serves the result, remuxed as-is when the device can play the
  codec and re-encoded when it can't. That
  covers internet services that need Clementine's credentials, CD audio,
  tracks cut from a cue sheet, formats the device can't decode, and audio
  that should carry Clementine's EQ and ReplayGain.

The main decision is that **Clementine stays in charge of playback, and the
device is only a renderer**. From the player's point of view, a remote device
is another audio output, much like choosing a different ALSA or PulseAudio
sink. That is why every existing feature keeps working without changes.

## 2. Goals and non-goals

Goals

- A user can pick "Play on: \<device\>" and the current and following tracks
  play there, with play/pause/seek/next/volume working from Clementine, from
  any remote client, and from MPRIS.
- Everything Clementine can play can be played remotely. Local files go
  directly. Everything else is restreamed.
- The official remote app can offer "Play on this phone", which registers the
  phone as a renderer.
- A controller and a renderer can be different devices. A phone can send
  playback to a tablet.
- Playback can move between outputs mid-track and keep its position.
- Service credentials (Authorization headers, OAuth tokens) never leave
  Clementine.
- The protocol stays backward compatible: old clients and old servers keep
  working.

Non-goals (for now)

- Keeping several renderers in sync (multi-room). The abstraction shouldn't
  rule it out, but it isn't designed here.
- Streaming outside the LAN. The existing `only_non_public_ip` policy applies.
- Replacing the remote's authentication or adding TLS. Section 9 notes where
  this design makes that easier later.

## 3. How the existing remote works

- `NetworkRemote` listens on TCP 5500 (configurable and advertised over
  Zeroconf as `_clementine._tcp`) and makes one `RemoteClient` per socket.
- Messages are a 4-byte length followed by a `cpb::remote::Message` (proto2,
  `version` defaults to 21). `IncomingDataParser` turns requests into Qt
  signals to `Player` and `PlaylistManager`. `OutgoingDataCreator` sends state
  (`CURRENT_METAINFO`, `ENGINE_STATE_CHANGED`, `UPDATE_TRACK_POSITION`, ...)
  to every client.
- Authentication is an optional numeric `auth_code` in `RequestConnect`.
- Downloads use a separate connection flagged `downloader`. `SongSender` sends
  whole files as protobuf `SONG_FILE_CHUNK`s and can transcode lossless files
  first with `Transcoder` presets (`NetworkRemote::kTranscoderSettingPostfix`).
- `Player` owns one `std::unique_ptr<EngineBase> engine_` (a `GstEngine`).
  About twenty call sites use `app_->player()->engine()` directly (analyzer,
  equalizer, seek slider, background streams). Two of them cast it to
  `GstEngine*` (`SettingsDialog` and the GStreamer debug console).
- `Player::HandleLoadResult` turns playlist URLs into a
  `MediaPlaybackRequest`: a media URL plus HTTP headers such as `Authorization`
  from Google Drive or OneDrive. `GstEnginePipeline` decodes it with
  `uridecodebin`, sends it through ReplayGain, EQ, balance and volume, then to
  the configured sink.

Two things follow from this:

1. The engine interface (`Engine::Base`: `Load`, `Play`, `Pause`, `Seek`,
   `StartPreloading`, `position_nanosec`, `state`, and the `TrackAboutToEnd`,
   `TrackEnded`, `StateChanged` and `Error` signals) is already a good fit for
   a remote renderer.
2. Media bytes should **not** go through the protobuf socket the way
   `SONG_FILE_CHUNK` does. Every media stack a renderer might use (ExoPlayer,
   AVPlayer, GStreamer, VLC, browsers, Chromecast, UPnP renderers) plays an
   HTTP URL with Range requests natively. HTTP also gives seeking and
   buffering for free.

## 4. Architecture

```
                      ┌─────────────────────────── Clementine ───────────────────────────┐
 Controller           │                                                                   │
 (any remote client) ─┼─ protobuf :5500 ─► IncomingDataParser ─► Player                   │
                      │                                            │ engine()             │
                      │                                            ▼                      │
                      │                                      EngineRouter  (Engine::Base) │
                      │                                       │          │                │
                      │                          active = local│          │active = remote│
                      │                                       ▼          ▼                │
                      │                                  GstEngine   RemoteEngine         │
                      │                                  (sound card)    │  uses          │
                      │                                                  ▼                │
                      │                                             StreamPlanner         │
                      │                                                  │ plan           │
                      │                                                  ▼                │
 Renderer             │                                            MediaHttpServer (:5500) │
 (phone, PC, ...)  ◄──┼─ protobuf :5500  RENDER_LOAD{url} ◄── RemoteEngine               │
                   ───┼─ HTTP :5500 GET /s/<token>/<item> ► Direct | Pipeline             │
                      └───────────────────────────────────────────────────────────────────┘
```

### 4.1 `EngineRouter`: output selection without touching `Player`

`EngineRouter` is a new `Engine::Base` that takes `GstEngine`'s place as
`Player::engine_`. It owns the local `GstEngine` and any number of
`RemoteEngine`s, forwards every call to the **active** one, and re-emits only
the active engine's signals.

Why a router rather than swapping `engine_` at runtime: the engine pointer is
used and connected to in many places (`MainWindow`, `BackgroundStreams`,
analyzer, equalizer, `NetworkRemote`). Swapping it would mean re-connecting
all of them. With a router, the pointer never changes, and those places need
no changes.

Rules:

- `SetOutput(OutputId)` is the only way to switch. To hand playback over, it
  reads `position_nanosec()` and `state()` from the old engine and stops it.
  It then calls `Load(current_req, Engine::Manual, ...)` on the new engine,
  followed by `Play(position)` if the old engine was playing, and emits
  `OutputChanged`.
- EQ, stereo balance, ReplayGain and volume settings go to **every** engine,
  so switching doesn't lose them. Each engine decides what it can apply.
- Background streams (`AddBackgroundStream`) always use the local
  `GstEngine`. They are ambience sounds, not part of the playlist.
- `scope()` comes from the active engine. A remote engine returns an empty
  scope unless it is using the Pipeline with Encode (§5.2), where it can feed buffers.
- The two `qobject_cast<GstEngine*>` call sites change to
  `router->local_engine()`.
- The selected output is saved in `QSettings` (`NetworkRemote/output`), but
  Clementine always **starts on local output**. A renderer that isn't
  connected can't be restored.

### 4.2 `RemoteEngine`: one per connected renderer

`RemoteEngine : Engine::Base` represents a single renderer. It turns engine
calls into protobuf commands and renderer status reports back into engine
signals.

| Engine call / signal         | Renderer protocol                                         |
|------------------------------|-----------------------------------------------------------|
| `Load(req, change, ...)`     | `StreamPlanner::Plan(req, song, caps)` → `RENDER_LOAD`    |
| `StartPreloading(req, ...)`  | plan → `RENDER_PRELOAD` (gapless where the renderer supports it) |
| `Play(offset)` / `Seek(pos)` | `RENDER_SEEK{offset}` (if non-zero) then `RENDER_PLAY` / `RENDER_SEEK{pos}` |
| `Pause` / `Unpause` / `Stop` | `RENDER_PAUSE` / `RENDER_PLAY` / `RENDER_STOP`            |
| `SetVolumeSW(percent)`       | `RENDER_SET_VOLUME`                                       |
| `position_nanosec()`         | last `RENDERER_STATUS.position_ms`, plus wall-clock time since it arrived while playing |
| `StateChanged`               | from `RENDERER_STATUS.state`                              |
| `TrackAboutToEnd`            | generated locally from the interpolated position (same logic as `EmitAboutToEnd`) |
| `TrackEnded`                 | `RENDERER_TRACK_ENDED` (with the item id, so late reports are ignored) |
| `Error` / `InvalidMediaRequested` | `RENDERER_ERROR`, by scope: an item error makes `Player` skip exactly as it does for local errors, a transient one reloads at the last position, a renderer error falls back to local output |
| `ValidMediaRequested`        | first `RENDERER_STATUS` in state Playing for that item id |

Crossfade and fade-out need two decoders mixed in one place, which a renderer
doesn't have. `RemoteEngine` turns them off (`is_autocrossfade_enabled()` and
friends return false) unless it is using live-mix mode (§5.4).

Losing the renderer (socket closed, or keep-alive timeout reusing
`OutgoingDataCreator`'s keep-alive) makes the router fall back to local
output, **paused** at the last reported position. Audio that suddenly starts
on the computer would be a bad surprise. `OUTPUTS` tells controllers
what happened.

### 4.3 `StreamPlanner`: choosing stream or restream

`StreamPlanner` is a pure function, so it is easy to unit-test. It decides how
to deliver each track:

```
Plan StreamPlanner::Plan(const MediaPlaybackRequest& req, const Song& song,
                         const RendererCaps& caps, const StreamSettings& s);

struct Plan {
  enum Mode { Direct, Pipeline };
  Mode mode;
  // Pipeline only: remux the source's own codec, or decode and encode.
  enum Output { Passthrough, Encode };
  Output output;
  TranscoderPreset preset;    // Encode only
  QString mime_type;          // what the renderer will receive
  qint64 length_nanosec;      // known length, used for seek bars and ?t=
};
```

There are only two modes, and two questions decide them.

**1. Can the file be served byte-for-byte (Direct)?** Only when all of these
hold:

- it is a local file, with no cue sheet and no begin/end markers,
- the renderer lists a format for its codec, and that format allows its
  sample rate (an empty list allows any),
- DSP isn't being applied remotely (`apply_dsp` off, or EQ and ReplayGain
  both off),
- it isn't lossless with `s.transcode_lossless` on (the setting downloads
  already use), and
- its bitrate isn't above `s.max_bitrate_kbps` (if set).

**2. Otherwise it goes through the Pipeline. Remux or encode?** Passthrough
when the renderer plays the source's codec at its sample rate *and* it has an entry
in the remux table (§5.2) *and* no DSP, markers or bitrate cap is in force.
Anything else is encoded: CD audio (already PCM), cue tracks, codecs or
sample rates the renderer can't play, codecs with no remux entry, DSP,
lossless-to-lossy and bitrate caps. The encoder's output is limited by the
target format's own entry: its sample rates become a caps filter, from which
`audioresample` picks the rate nearest the source's, and its channel limit
likewise.

So internet services, Google Drive files and radio usually end up as
Pipeline + Passthrough: the same bytes, fetched by Clementine with
Clementine's credentials.

Handing a renderer the raw upstream URL of a header-less stream stays an
opt-in (`s.allow_raw_urls`, off by default), because it reveals the URL and
loses stream titles and control of the stream.

"Stream" in the request means Direct. "Restream" means Pipeline, in either
of its outputs.

## 5. Delivery modes

Both modes are served by `MediaHttpServer`, a small HTTP/1.1 handler that
shares the remote's existing port (5500 by default) with the protobuf
protocol. That means one port to open in a firewall, one Zeroconf record,
and the same listen addresses and `only_non_public_ip` check with no extra
code.

### 5.0 Sharing the port

`NetworkRemote::AcceptConnection` already runs the `only_non_public_ip` check.
After that, instead of making a `RemoteClient` straight away, it waits for the
first byte from the socket and **peeks** at it without consuming it
(`QTcpSocket::peek`):

- The protobuf framing starts with a big-endian `quint32` length
  (`QDataStream`'s default byte order). `RemoteClient::IncomingData` already
  disconnects any client whose length is over 128 MiB (`0x08000000`), so a
  valid first byte is always `0x00`–`0x08`.
- Every HTTP/1.x method (`GET`, `HEAD`, ...) starts with an upper-case ASCII
  letter, `0x41`–`0x5A`.

The two ranges don't overlap, so this isn't a guess based on low odds: a
connection whose first byte is a letter is one the protobuf protocol would
already reject. One byte decides it, and no buffering is needed. First byte
`0x00`–`0x08` goes to `RemoteClient` as today. `A`–`Z` goes to
`MediaHttpServer` if streaming is enabled. Anything else, or HTTP while
streaming is off, closes the socket.

The protocols are client-speaks-first: neither the remote nor HTTP sends
anything before the client does, so waiting for the first byte changes
nothing for existing clients. A connection that sends nothing within 10 s is
closed, which also stops idle sockets from piling up before they are
classified.

The same byte range leaves room for TLS later: a TLS ClientHello starts with
`0x16`, which matches neither protocol, so it can be recognised the same way.

The HTTP side speaks HTTP/1.1 only. Appendix A explains why HTTP/2 isn't
supported and how an HTTP/2 client is handled.

The whole mechanism is a small `ProtocolSniffer` step in `NetworkRemote`.
`RemoteClient` and `MediaHttpServer` each receive a socket that belongs to
them and never see the other protocol. `MediaHttpServer` has no
`QTcpServer` of its own; it takes sockets through `HandleConnection(QTcpSocket*)`.

URLs are opaque: `http://<host>:<remote port>/s/<session-token>/<item-id>[?t=<ms>]`.
`item-id` is a key into a table held by the `RemoteEngine`. The table maps it
to `{Plan, MediaPlaybackRequest, Song}`. Entries are made on `RENDER_LOAD` or
`RENDER_PRELOAD`, and at most the current and next items are kept. **No file
path or upstream URL ever appears in a streaming URL.** Clients can't
enumerate the library through this server, and path traversal isn't possible.

### 5.1 Direct: streaming local files

- `QFile` with `Range`/`206 Partial Content`, `Content-Length`, the
  `Content-Type` from the file type, and `Accept-Ranges: bytes`.
- Seeking and buffering happen on the renderer. `RENDER_SEEK` just tells it
  where to go.
- Gapless playback: `RENDER_PRELOAD` is sent `kPreloadGapNanosec` before the
  end, as `GstEngine` does today. The renderer queues it (ExoPlayer
  `ConcatenatingMediaSource`, AVQueuePlayer, GStreamer `about-to-finish`).
- Reads happen in chunks driven by `bytesWritten`. The whole file is never
  loaded into memory, unlike `SongSender::SendSingleSong`.

### 5.2 Pipeline: restreaming everything else

Everything that isn't Direct goes through one kind of pipeline. A new
`GstStreamPipeline : GstPipelineBase` is built for each request:

```
uridecodebin(uri = req.MediaUrl, caps = raw | <passthrough caps>)
  ├─ Passthrough: <parser> ! <muxer> ! appsink
  └─ Encode:      audioconvert ! [rgvolume ! rglimiter] ! [equalizer-nbands ! audiopanorama]
                  ! audioresample ! capsfilter(rate/channels <= caps)
                  ! <preset encoder> ! <preset muxer> ! appsink
```

`uridecodebin`'s `caps` property sets the format at which it stops decoding.
With the source's compressed caps added to it, `uridecodebin` hands over
parsed but undecoded audio, so Passthrough costs about as much CPU as a
proxy. There is one pipeline and one response path, with two tails.

**Fetching the source.** The pipeline sets up its source with the same
`SourceSetupCallback` logic as `GstEnginePipeline` (moved into a shared
helper). That code already sets `extra-headers` (for example the
`Authorization` header `UrlHandler::LoadResult::auth_header_` provides) and
`user-agent` on the source element. The consequences:

- Credentials stay in Clementine, with no separate HTTP relay to write.
- Seeking in a remote file works: the HTTP source sends its own Range
  requests upstream when the pipeline seeks.
- Radio stream titles (ICY and in-stream tags) come out of the pipeline as
  tags and go into `MetaData`, just as they do for local playback.
- A short-lived signed URL (Subsonic's or Plex's `UrlHandler`) that returns
  401 or 403 mid-stream is resolved again through `Player`'s `UrlHandler`,
  and the pipeline is rebuilt once at the current position.

**Passthrough: the remux table.** Passthrough has to produce a container a
renderer can play as a stream:

| Source codec | Parser → output | MIME type |
|---|---|---|
| MP3 | `mpegaudioparse` → raw MP3 | `audio/mpeg` |
| AAC (MP4/M4A or ADTS) | `aacparse` → ADTS | `audio/aac` |
| FLAC | `flacparse` → raw FLAC | `audio/flac` |
| Vorbis, Opus | `vorbisparse`/`opusparse` → `oggmux` | `audio/ogg` |
| anything else | — | Encode instead |

Remuxing can drop the gapless information some containers carry (MP4's
`iTunSMPB`, the LAME/Xing header). That is acceptable here, because it only
happens for sources that couldn't be served Direct anyway. It is also why
local files the renderer can play are never remuxed (§5.1).

**Encode.** The encoder and muxer come from the **existing
`TranscoderPreset`s**, the ones already offered for downloads (Ogg
Vorbis/Opus, MP3, AAC/M4A, FLAC), so there are no new format lists to
maintain. The default is Opus in Ogg at 128 kbps when the renderer lists it,
otherwise MP3 at 192 kbps. The ReplayGain, EQ and balance elements are made
with the same helper `GstEnginePipeline::InitAudioBin` uses. That code moves
into a shared function so local and remote playback sound the same. Begin
and end markers (cue tracks) become a segment seek on the decoder, the same
way `GstEnginePipeline` handles them.

**Common to both outputs:**

- The output has no known length, so seeking uses `?t=<ms>`. The renderer
  gets `RENDER_SEEK{position, url}` with a new URL, and the server starts a
  new pipeline that seeks to `t` before going to PLAYING. The response is
  chunked, with no `Content-Length` and `Accept-Ranges: none`. Length comes
  from `Song::length_nanosec` in `RENDER_LOAD`, so the renderer's seek bar
  is still correct. Live radio simply has no length.
- `appsink` runs with `sync=false`, so a file-backed pipeline can run faster
  than real time. The renderer can buffer ahead, but not as freely as with
  Direct (§5.1): the pipeline pauses when it is more than 60 s of audio ahead
  of what the socket has sent.
- Resources are bounded: at most one active pipeline per renderer, plus one
  to preload the next track.
- The Encode tail can optionally tee raw audio into `BufferConsumer`, so the
  analyzer and moodbar in Clementine's window keep moving while it plays
  remotely.

**Why Direct is still a separate mode.** A file served byte-for-byte has a
`Content-Length` and Range support, so the renderer seeks instantly without
asking Clementine and can buffer the whole track quickly. That matters on
phones, where a Wi-Fi drop or the laptop sleeping mid-track shouldn't stop
playback. The original file also keeps its gapless and duration metadata,
is bit-perfect, and is what DLNA renderers (Phase 3) handle best. It is also
the most common case, and it costs a `QFile` plus Range parsing.

### 5.4 Live mix (later phase)

This mode keeps crossfades, fade-outs and one continuous stream across
tracks. The engine's audio bin ends in `encoder ! muxer ! appsink` instead of
`audiosink`, and every renderer gets one unending HTTP stream (like Icecast).
It uses the same `set_output_device` path in `GstEnginePipeline`, plus an
output option for "remote mixed stream".

Costs: 2–5 s of latency between a control action and what the user hears,
position that has to be adjusted for the renderer's buffer, and no native
seeking on the renderer. It is worth having for "party mode", but it
shouldn't be the default. Direct and Pipeline are Phase 1 and 2.

## 6. Protocol changes

All changes add to the existing messages, so existing clients and servers are
unaffected. `Message.version` goes from 21 to 22. Clients check for
`SERVER_FEATURE_RENDERING` in `ResponseClementineInfo.features` before using
any of it, and the server only offers rendering to clients whose
`RequestConnect` includes `renderer`.

### 6.1 Conventions for new definitions

New definitions follow buf's `STANDARD` lint rules where they apply to a
proto2 file with no services, plus two house rules:

- **Enums, not bools.** A bool can only ever mean two things. Anything that
  might grow a third answer is an enum, and a set of on/off capabilities is a
  `repeated` enum of features. A new capability is then a new enum value,
  not a new field that every client has to learn about.
- **The first value of every enum is `<ENUM_NAME>_UNSPECIFIED = 0`**
  (`ENUM_FIRST_VALUE_ZERO`, `ENUM_ZERO_VALUE_SUFFIX`). In proto2 this
  matters even more than in proto3: an `optional` enum field that is unset,
  *or* that holds a value the reader's older copy of the schema doesn't
  know, reads back as the first declared value. With `_UNSPECIFIED` first,
  "not sent" and "sent something I don't understand" both land on a value
  that means "unknown", never on a real choice made by accident. Readers
  treat `_UNSPECIFIED` as "use the safe default".
- **Every value carries its enum's name as a prefix** (`ENUM_VALUE_PREFIX`),
  in `UPPER_SNAKE_CASE`. Enum values share the package's scope in C++, and
  the file already has top-level names like `UNKNOWN`, `Playing` and
  `Idle`, so unprefixed new values would sooner or later collide.
- **No bare scalars in `Message`.** Each new payload is its own message,
  even when it holds a single field today, so it can grow without a new
  `Message` field.
- **Units in field names** (`_ms`, `_hz`, `_kbps`), `lower_snake_case`
  fields, `PascalCase` messages and enums, and a leading comment on every new
  message, enum and value (buf's `COMMENTS` category).
- Field numbers are never reused. A removed field or value is `reserved`.

**The one exception is `MsgType`.** Its existing values have no prefix and
aren't renamed here, because renaming generated identifiers would break
every client's source even though the wire format wouldn't change. New
`MsgType` values follow the existing style so the enum stays consistent;
`UNKNOWN = 0` already plays the part of its `_UNSPECIFIED`. The same goes for
the other existing enums (`EngineState`, `RepeatMode`, ...): they are left
alone, and new code doesn't reuse them where a new enum fits better (see
`RendererState`).

### 6.2 Definitions

```proto
enum MsgType {
  // ...existing values...

  // Renderer -> server. A renderer registers by setting
  // RequestConnect.renderer, and unregisters by disconnecting.
  RENDERER_STATUS       = 302;  // periodic (1 Hz while playing) + on change
  RENDERER_TRACK_ENDED  = 303;
  RENDERER_ERROR        = 304;
  REQUEST_OUTPUTS       = 305;
  SET_OUTPUT            = 306;  // any authenticated controller may send

  // Server -> renderer
  RENDER_LOAD           = 320;
  RENDER_PRELOAD        = 321;
  RENDER_PLAY           = 322;
  RENDER_PAUSE          = 323;
  RENDER_STOP           = 324;
  RENDER_SEEK           = 325;
  RENDER_SET_VOLUME     = 326;

  // Server -> all clients, whenever an output is added or removed or the
  // active one changes. Also the reply to REQUEST_OUTPUTS.
  OUTPUTS               = 340;
}

// Optional abilities of the server, sent in ResponseClementineInfo.
enum ServerFeature {
  SERVER_FEATURE_UNSPECIFIED = 0;
  // The server can send playback to renderers (streaming is enabled).
  SERVER_FEATURE_RENDERING = 1;
}

// Optional abilities of a renderer, beyond decoding its formats.
enum RendererFeature {
  RENDERER_FEATURE_UNSPECIFIED = 0;
  // Can queue a RENDER_PRELOAD item and start it without a gap.
  RENDERER_FEATURE_GAPLESS = 1;
  // Sends HTTP Range requests, so it can seek in Direct streams itself.
  RENDERER_FEATURE_HTTP_RANGE = 2;
}

// One format a renderer can decode, with its limits.
message AudioFormat {
  // "audio/flac", "audio/ogg; codecs=opus", "audio/mpeg". Without codecs=,
  // any codec in that container. Strings rather than an enum, because the
  // set is open-ended and already standardised.
  optional string mime_type = 1;
  // The sample rates it plays. Empty: any rate the format allows.
  repeated int32 sample_rates_hz = 2;
  // Unset or 0: as many channels as the format allows.
  optional int32 max_channels = 3;
}

// What a renderer can play. Sent when it registers.
message RendererCapabilities {
  // Stable per install (a UUID), so a renderer is recognised after reconnecting.
  optional string renderer_id = 1;
  // Shown in output pickers: "Pixel 9", "Kitchen tablet".
  optional string display_name = 2;
  // The formats it can decode, in no particular order.
  repeated AudioFormat formats = 3;
  // Unknown values are ignored by the server.
  repeated RendererFeature features = 4;
  // Upper limit the renderer wants, for example on mobile data. Unset or 0
  // means no limit.
  optional int32 max_bitrate_kbps = 5;
}

message RequestConnect {
  // ...existing fields 1..3...
  // Present when the client also wants to act as a renderer.
  optional RendererCapabilities renderer = 4;
}

message ResponseClementineInfo {
  // ...existing fields 1..4...
  // Unknown values are ignored by clients.
  repeated ServerFeature features = 5;
}

// Whether an output can be used right now.
enum OutputState {
  OUTPUT_STATE_UNSPECIFIED = 0;
  // Connected and can be chosen.
  OUTPUT_STATE_AVAILABLE = 1;
  // Playback is being handed over to it.
  OUTPUT_STATE_ACTIVATING = 2;
  // The current output.
  OUTPUT_STATE_ACTIVE = 3;
}

// One place playback can go: this computer or a renderer.
message Output {
  // "local" for this computer, otherwise the renderer_id.
  optional string output_id = 1;
  optional string display_name = 2;
  optional OutputState state = 3;
}

// Every output, sent with OUTPUTS.
message ResponseOutputs {
  repeated Output outputs = 1;
}

// A controller asks for playback to move to another output.
message RequestSetOutput {
  optional string output_id = 1;
}

// How the server delivers an item. Informational for the renderer; how to
// seek is given separately by SeekMethod.
enum StreamMode {
  STREAM_MODE_UNSPECIFIED = 0;
  // The original file, byte for byte.
  STREAM_MODE_DIRECT = 1;
  // Output of a GStreamer pipeline, remuxed or encoded.
  STREAM_MODE_PIPELINE = 2;
}

// How a renderer seeks within an item.
enum SeekMethod {
  // Treat as SEEK_METHOD_NONE.
  SEEK_METHOD_UNSPECIFIED = 0;
  // Not seekable, for example live radio.
  SEEK_METHOD_NONE = 1;
  // Seek within the current URL using HTTP Range requests.
  SEEK_METHOD_BYTE_RANGE = 2;
  // Load the url given in RequestRenderSeek.
  SEEK_METHOD_NEW_URL = 3;
}

// One track, as the renderer should fetch and present it.
message RenderItem {
  optional int32 item_id = 1;
  // http://host:5500/s/<token>/<item_id>
  optional string url = 2;
  optional string mime_type = 3;
  optional StreamMode mode = 4;
  // Unset for items with no known length, such as radio.
  optional int64 length_ms = 5;
  optional SeekMethod seek_method = 6;
  // For the lock screen and notification UI.
  optional SongMetadata song = 7;
}

// What a renderer does once an item has loaded.
enum LoadStartState {
  // Treat as LOAD_START_STATE_PAUSED.
  LOAD_START_STATE_UNSPECIFIED = 0;
  LOAD_START_STATE_PAUSED = 1;
  LOAD_START_STATE_PLAYING = 2;
}

// Load an item now, replacing whatever is playing.
message RequestRenderLoad {
  optional RenderItem item = 1;
  // Where to start. For SEEK_METHOD_NEW_URL items, item.url already starts
  // there; for SEEK_METHOD_BYTE_RANGE items the renderer seeks to it.
  optional int64 start_ms = 2;
  optional LoadStartState start_state = 3;
}

// Queue the item that follows the current one.
message RequestRenderPreload {
  optional RenderItem item = 1;
}

message RequestRenderSeek {
  optional int32 item_id = 1;
  optional int64 position_ms = 2;
  // Set when the item's seek_method is SEEK_METHOD_NEW_URL.
  optional string url = 3;
}

message RequestRenderVolume {
  // 0..100
  optional int32 volume = 1;
}

// A renderer's playback state. Separate from EngineState because a renderer
// has states the local engine doesn't report, such as buffering.
enum RendererState {
  RENDERER_STATE_UNSPECIFIED = 0;
  RENDERER_STATE_IDLE = 1;
  RENDERER_STATE_LOADING = 2;
  RENDERER_STATE_BUFFERING = 3;
  RENDERER_STATE_PLAYING = 4;
  RENDERER_STATE_PAUSED = 5;
}

// Sent at 1 Hz while playing, and whenever the state changes.
message RendererStatus {
  optional int32 item_id = 1;
  optional RendererState state = 2;
  optional int64 position_ms = 3;
  optional int32 buffered_percent = 4;
}

// The item finished playing to its end.
message RendererTrackEnded {
  optional int32 item_id = 1;
}

// What an error affects, and so what the server does about it.
enum RendererErrorScope {
  // Treat as RENDERER_ERROR_SCOPE_ITEM.
  RENDERER_ERROR_SCOPE_UNSPECIFIED = 0;
  // This item can't be played. Retry through the pipeline or skip it.
  RENDERER_ERROR_SCOPE_ITEM = 1;
  // A temporary problem, such as a network drop. Reload at the last position.
  RENDERER_ERROR_SCOPE_TRANSIENT = 2;
  // The renderer can't continue. Fall back to local output.
  RENDERER_ERROR_SCOPE_RENDERER = 3;
}

message RendererError {
  optional int32 item_id = 1;
  optional string message = 2;
  optional RendererErrorScope scope = 3;
}

message Message {
  // ...existing fields...
  optional RendererCapabilities renderer_capabilities = 60;
  optional ResponseOutputs response_outputs = 61;
  optional RequestSetOutput request_set_output = 62;
  optional RequestRenderLoad request_render_load = 63;
  optional RequestRenderPreload request_render_preload = 64;
  optional RequestRenderSeek request_render_seek = 65;
  optional RequestRenderVolume request_render_volume = 66;
  optional RendererStatus renderer_status = 67;
  optional RendererError renderer_error = 68;
  optional RendererTrackEnded renderer_track_ended = 69;
}
```

Every `_UNSPECIFIED` has a stated meaning, so a reader never has to guess:
the comment on the value says what to do (for example, an unspecified
`LoadStartState` stays paused, which is the safe choice). Where no comment
says otherwise, `_UNSPECIFIED` means the sender didn't say, and the message is
handled as if the field were absent.

### 6.3 Notes

- **Limits are per format.** Real limits depend on the codec (a renderer may
  play FLAC only up to 96 kHz while playing AAC at any rate), which is also
  how browsers (`MediaCapabilities.decodingInfo`) and Android
  (`MediaCodecInfo`) describe them, so each `AudioFormat` carries its own.
  The limits stop at sample rate and channels because Clementine's `Song`
  records neither channel count nor bit depth, so bit depth couldn't be
  checked yet; a `max_bits_per_sample` field can be added later without
  breaking older clients.
- **Sample rates are a list of integers, not an enum or a maximum.** A list
  records gaps (a device that plays 48 kHz but not 44.1 kHz) and gives the
  encoder rates to choose from. Rates are an open set of numbers, so an enum
  would need a schema change for each new one and would say nothing the
  number doesn't. One entry per format, rather than one per combination of
  format, rate and channel count, says the same thing without multiplying
  entries.

- The renderer uses its **existing control connection**, so authentication,
  keep-alive and disconnect handling work as they do now. A device can be a
  controller and a renderer at once. The "Play on this phone" button just
  sends `SET_OUTPUT{own renderer_id}`.
- `item_id` is on every command and report. It prevents a status arriving
  after a track change from being applied to the wrong track.
- `OUTPUTS` is sent to every client when a renderer joins or leaves, so every
  controller's output picker stays up to date.
- Existing `UPDATE_TRACK_POSITION`, `ENGINE_STATE_CHANGED` and
  `CURRENT_METAINFO` still go to all clients. They are driven by
  `EngineRouter`, so controllers show the remote renderer's position without
  any changes. `RemoteEngine` maps `RendererState` onto `Engine::State` for
  them (loading and buffering count as playing).
- To check the new definitions, CI can run `buf lint` with `STANDARD` and
  `COMMENTS` on the proto file, with the existing definitions listed in
  `ignore_only` so only new ones are held to the rules. The file's location
  also fails `PACKAGE_DIRECTORY_MATCH` (package `cpb.remote` in
  `ext/libclementine-remote/`); moving it is out of scope, so that rule is
  excluded too.

## 7. Code layout

```
src/engines/enginerouter.{h,cpp}           EngineRouter (Engine::Base)
src/engines/gstpipelinehelpers.{h,cpp}     source setup (headers, user agent) and RG/EQ/balance elements, shared by GstEnginePipeline and GstStreamPipeline
src/networkremote/streaming/
    remoteengine.{h,cpp}                   RemoteEngine (Engine::Base)
    rendererregistry.{h,cpp}               renderer_id -> RemoteEngine; owns OUTPUTS
    streamplanner.{h,cpp}                  pure planning logic
    protocolsniffer.{h,cpp}                first-byte routing of accepted sockets (§5.0)
    mediahttpserver.{h,cpp}                HTTP request parsing, token check, dispatch
    directresponder.{h,cpp}                file + Range
    pipelineresponder.{h,cpp}              owns a GstStreamPipeline, appsink -> socket
    gststreampipeline.{h,cpp}              GstPipelineBase subclass
src/ui/outputpicker.{h,cpp}                toolbar button + menu: "This computer", renderers
```

Changes to existing files:

- `Player`: construct an `EngineRouter` around `GstEngine`. Nothing else in
  `Player` changes.
- `NetworkRemote`: own `RendererRegistry` and `MediaHttpServer`, and route
  each accepted socket by its first byte (§5.0) instead of always making a
  `RemoteClient`.
- `RemoteClient`: store `RendererCapabilities` from `CONNECT` and issue the
  session token.
- `IncomingDataParser`: handle the new message types and pass them to the
  registry.
- `OutgoingDataCreator::SendClementineInfo`: add `SERVER_FEATURE_RENDERING` to
  `features`.
- `SettingsDialog`, `MainWindow`: `qobject_cast<GstEngine*>` becomes
  `EngineRouter::local_engine()`.
- `networkremotesettingspage.ui`: add a "Streaming" group (§8).

## 8. User interface

- **Output picker**: a button next to the volume slider, shown only when the
  remote is enabled, streaming is allowed, and at least one renderer is
  connected. Its menu lists "This computer" and every renderer, with a check
  mark on the active one. The tray menu and MPRIS stay as they are.
- **Settings → Network Remote → Streaming**
  - *Allow playing on remote devices* (off by default. It exposes media the
    same way *Allow downloads* does, so it follows the same idea.)
  - *When the device can't play a file, convert to:* \[transcoder preset\]
    (reuses the preset list downloads already show)
  - *Always convert lossless files* (shares the setting with downloads)
  - *Apply equalizer and ReplayGain on remote devices* (forces the Pipeline to encode)
  - *Maximum bitrate* (none / 320 / 192 / 128 kbps)
- The status bar shows "Playing on \<device\>" while a remote output is
  active.

## 9. Security

- **Off by default.** It needs the remote to be enabled *and* the new
  setting to be on.
- **Same network policy.** HTTP arrives on the remote's own sockets, so the
  listen addresses and `IpIsPrivate` check apply before it is even
  recognised as HTTP. With streaming off, HTTP connections are closed
  unanswered, so the port looks exactly as it does today. Every request's peer address must also
  match the peer address of the control connection that owns the token.
- **Capability URLs.** The session token is 128 bits from
  `QRandomGenerator::system()`, issued only after `RemoteClient` is
  authenticated. It is revoked when that connection closes. Item ids only
  resolve within their own session, and only for the current and next
  items. Without a live authenticated control connection, the HTTP server
  serves nothing.
- **Credentials stay in Clementine.** In Pipeline mode the renderer
  gets Clementine's URL, never the upstream URL or its `Authorization`
  header. Raw-URL handoff (`allow_raw_urls`) only applies to URLs without
  headers, and it is off by default.
- **No path exposure.** Files are chosen by `item_id`, never by path. That
  sidesteps the whole class of traversal bugs `files_root_folder` handling
  has to guard against.
- **Resource limits.** A cap on concurrent HTTP responses per session (4), on
  pipelines per renderer (2) and on header size (8 KiB). Idle sockets time
  out after 30 s.
- **Known weakness, unchanged.** The remote's auth code is a short number
  sent in plaintext. This design doesn't make that worse. Tokens are only
  handed out on authenticated connections. TLS could be added later on the
  same port, since its first byte is distinguishable too (§5.0).

## 10. Edge cases

| Case | Behaviour |
|------|-----------|
| Renderer disconnects mid-track | Fall back to local, paused at the last position. Send `OUTPUTS` |
| Renderer reports a track-level error | `Error` + `InvalidMediaRequested`. `Player` skips to the next track, as it does locally |
| The same track fails Direct because the codec was advertised but doesn't actually decode | Renderer sends `RENDERER_ERROR` with `RENDERER_ERROR_SCOPE_ITEM`. `RemoteEngine` tries the same item once more through the Pipeline with Encode, before reporting an error |
| Stream metadata changes (radio) | The Pipeline emits the tags into `MetaData`, so titles update as they do for local playback |
| Clementine quits while playing remotely | `RENDER_STOP`, then the usual `DISCONNECT{Server_Shutdown}` |
| Two controllers choose different outputs | Last write wins. Everyone gets `OUTPUTS` |
| Hand-off during Pipeline playback | New `RENDER_LOAD` with `start_ms`. The server makes a fresh pipeline seeked to that point |
| Stop after current, repeat, shuffle, queue | Unchanged. They are `Player`/`PlaylistSequence` logic above the engine |
| Scrobbling / play counts | Unchanged. They use `Player`'s position and state signals, which the router drives |

## 11. Phasing

1. **Phase 1: streaming local files.** Protocol v22, `EngineRouter`,
   `RemoteEngine`, `RendererRegistry`, `MediaHttpServer` with Direct only,
   `StreamPlanner`'s Direct check only (tracks that would need the Pipeline
   are skipped with a clear error), output picker, settings. The Android remote gets "Play on this
   phone". This version is already useful for most local libraries.
2. **Phase 2: restreaming.** Pipeline mode with both outputs: Passthrough
   (internet services, radio) and Encode (cue, CDDA, unsupported codecs,
   lossless and bitrate caps,
   optional DSP), `?t=` seeking, sharing the audio-bin helper.
3. **Phase 3: more renderers and live mix.** Because delivery is plain HTTP,
   UPnP/DLNA MediaRenderers (`AVTransport.SetAVTransportURI`) and Chromecast
   (Default Media Receiver `LOAD`) are just more `RemoteEngine` backends
   behind an `IRendererTransport` interface. They are discovered with SSDP or
   mDNS rather than registering themselves. Live-mix mode (§5.4) comes here
   too.

## 12. Testing

- `streamplanner_test.cpp`: a table-driven test of the Direct check and the
  Passthrough/Encode choice in §4.3. It is
  pure logic and needs no GStreamer or network.
- `mediahttpserver_test.cpp`: Range parsing (`bytes=0-`, `bytes=100-199`,
  suffix ranges, invalid ranges → 416), token and peer checks, unknown item
  → 404, a Direct round trip against a temporary file over `127.0.0.1`.
- `enginerouter_test.cpp`: uses `tests/mock_engine.h` for both backends and
  checks call forwarding, signal filtering, and hand-off order (stop old →
  load new with position → play).
- `remoteengine_test.cpp`: a fake renderer over a local socket speaking the
  new messages. Covers position interpolation, stale `item_id` reports being
  ignored, and disconnect falling back to local.
- Extend `networkremote_test.cpp` to check that `SERVER_FEATURE_RENDERING` is
  only advertised when enabled.
- `protocolsniffer_test.cpp`: first byte `0x00`–`0x08` goes to the remote,
  `A`–`Z` to HTTP only when streaming is on, anything else is closed; a
  first byte that arrives in a separate TCP segment from the rest; the
  10 s silence timeout.
- Pipeline: GStreamer tests through `GstStreamPipeline`. Encode turns a short
  WAV fixture into Ogg/Opus. Passthrough turns MP3 and FLAC fixtures into
  their raw forms, and the output decodes to the same PCM as the input.
  Both check the caps and approximate duration, including a `?t=` start.

## 13. Open questions

1. Should Clementine's volume slider set the renderer's device volume, or a
   software gain inside the Pipeline? The proposal is the renderer's volume
   (`RENDER_SET_VOLUME`), because Direct mode can't apply gain.
2. Should the choice of output be per playlist ("the kitchen playlist always
   plays in the kitchen")? It's out of scope, but `EngineRouter::SetOutput`
   should be callable from `PlaylistManager` later.
3. Should a renderer be able to *pull* the next track itself (a playlist
   cursor on the renderer) to survive short losses of the control connection
   on mobile? This design keeps the renderer passive. That is simpler and
   keeps one source of truth, at the cost of a gap if the control connection
   is down exactly when a track ends. `RENDER_PRELOAD` reduces the risk.

## 14. Prototype

A prototype of Phases 1 and 2 is on this branch, with a Python command line
client in `tools/remote-cli` that can act as a renderer or a controller.
Where it differs from the design above:

- **Threads.** `NetworkRemote`, its `RemoteClient`s and `MediaHttpServer`
  run on the network remote's own thread, while the `Player` and its
  engines run on the main thread. `RendererRegistry` and the
  `RemoteEngine`s live on the main thread and talk to the network thread
  through queued signals carrying serialized messages and client ids.
  `StreamItemTable` (mutex-guarded) is how the HTTP side sees which items a
  token may fetch.
- **Registration.** There are no `REGISTER_RENDERER`/`UNREGISTER_RENDERER`
  or `OUTPUT_CHANGED` messages: a client registers with
  `RequestConnect.renderer`, unregisters by disconnecting, and `OUTPUTS` is
  broadcast on every change.
- **HTTP.** One request per connection (`Connection: close`) rather than
  keep-alive. HTTP/2 clients get `400`.
- **Encoders.** The Pipeline's encoders are a short fixed list in
  `StreamPlanner` (Opus in Ogg, MP3, Vorbis in Ogg), not the transcoder
  presets. Passthrough is only chosen when the song's file type is known,
  which internet services often don't set.
- **Not done yet.** Applying EQ and ReplayGain on remote output, the output
  picker in Clementine's own window, the Pipeline's tee into the
  analyzer, and resource limits per session. Handing paused playback to the
  local engine plays for a moment before it pauses, because `GstEngine` can't
  load paused.

Tested end to end against a Clementine with an isolated configuration:
Direct streaming of MP3, FLAC and Ogg with Range seeks; Pipeline encoding
FLAC and Vorbis to MP3, with `?t=` seeks; the remux descriptions for MP3,
FLAC, Vorbis, Opus and AAC (checked with `gst-launch`); pause and resume;
moving playback to the local output and back at the same position; falling
back to local, paused, when the renderer disconnects; gapless preloading;
and the `ffplay` renderer.

## Appendix A: HTTP/2

`MediaHttpServer` speaks HTTP/1.1 with keep-alive. It doesn't support
HTTP/2, either over TLS (h2) or in cleartext (h2c).

**Almost no renderer would use h2c.** Most clients only negotiate HTTP/2
during a TLS handshake:

- Browsers don't implement h2c at all.
- Apple's `AVPlayer`/`URLSession`, Chromecast and GStreamer's `souphttpsrc`
  (libsoup) only use HTTP/2 over TLS.
- On Android, ExoPlayer's default HTTP stack is HTTP/1.1. OkHttp can use h2c
  only when told in advance that the server supports it, with no fallback,
  so a client would have to opt in deliberately.
- DLNA renderers (Phase 3) are HTTP/1.1.

**HTTP/2's benefits don't apply to this traffic.** Each renderer has at most
two long responses at once (the current and the preloaded item), each
several MB. Multiplexing and header compression gain nothing there, and
HTTP/2 over TCP has the same head-of-line blocking as HTTP/1.1. The one real
gain is cheaper seek cancellation: HTTP/2 can cancel a single request and
keep the connection, where HTTP/1.1 closes it. On a LAN that saves one TCP
handshake per seek, which is negligible.

**The costs are real:**

- HTTP/2 has its own per-stream and per-connection flow control. It would
  sit on top of the back-pressure in §5.1 and §5.2, giving two sets of
  windows to tune against each other.
- Qt Network has an HTTP/2 client but no server, and nothing in Clementine's
  current dependencies provides one. Supporting it would mean adding nghttp2
  or a new Qt module, plus framing, header compression and stream state, to
  replace a few hundred lines of HTTP/1.1.

**How HTTP/2 clients are handled:**

- An h2c client that assumes support opens with the fixed preface
  `PRI * HTTP/2.0`. `P` is in the `A`–`Z` range, so §5.0 routes it to the
  HTTP handler, which closes any connection that starts with `PRI`. Such a
  client can't fall back to HTTP/1.1 on the same connection, so there is
  nothing useful to reply.
- An `Upgrade: h2c` header on an HTTP/1.1 request is ignored, and the
  response is plain HTTP/1.1. RFC 9113 deprecated that upgrade path.

**When to revisit.** If TLS is added later (§5.0), HTTP/2 comes almost for
free through ALPN, as long as the TLS stack already provides it. That is the
point to reconsider, although the gain for this traffic would still be small.

## Appendix B: a web client

This appendix isn't part of the plan. It records what a browser-based
remote would involve, so the choices above don't rule it out.

### Transport

Browsers can't open raw TCP, so a web client needs a WebSocket. It fits the
shared port (§5.0) without a new port:

- The HTTP handler reads the request line and headers with `peek`, without
  consuming them, and routes by path: `/api/ws` to a WebSocket, `/s/...` to
  media, `/` to the web app.
- For `/api/ws` it hands the untouched socket to
  `QWebSocketServer::handleConnection(QTcpSocket*)`, which performs the
  handshake. This adds a dependency on `Qt6::WebSockets`.
- Each WebSocket message carries one existing `cpb::remote::Message`.
  WebSocket already delimits messages, so the 4-byte length prefix is
  dropped. The browser decodes it with a proto2-capable library such as
  protobuf-es.
- `RemoteClient` gets a transport interface with two implementations, the
  existing framed TCP and WebSocket. `IncomingDataParser`,
  `OutgoingDataCreator`, authentication, keep-alive and the renderer
  messages don't change.

### Serving the app

Clementine has to serve the web app itself, from Qt resources at `/`:

- **Mixed content.** A page served over HTTPS (for example a hosted copy on
  GitHub Pages) isn't allowed to open a plain `ws://` connection, so a hosted
  client would need TLS on Clementine first.
- **Same origin.** Served from Clementine, the app shares an origin with the
  WebSocket and media URLs. No CORS is needed, and the Origin check below is
  a simple equality test.
- **Versions.** The app always matches the server's protocol version.
- **Discovery.** Browsers can't do mDNS. The user types `host:5500`, or scans
  a QR code shown on the remote settings page.

### The browser as a renderer

An `<audio>` element can play both delivery modes: Direct with Range
requests, and the Pipeline's chunked responses. `canPlayType()`, or
`MediaCapabilities.decodingInfo()` for sample rates and channels, fills in
`RendererCapabilities.formats`. This gives "Play on this browser" with
nothing to install, and is the easiest way to test streaming end to end.
Browsers bring their own limits, though:

- **Autoplay.** Browsers block audio that starts without a user gesture on
  the page. A `RENDER_LOAD` with `LOAD_START_STATE_PLAYING` sent from another
  controller fails until the user has interacted with the browser tab. The
  page must unlock playback on the first click (the "Play on this browser"
  button does that), and report `RENDERER_ERROR_SCOPE_TRANSIENT` with a clear
  message when a play attempt is refused.
- **No gapless playback.** `<audio>` leaves a gap between tracks, so the
  renderer must not declare `RENDERER_FEATURE_GAPLESS`. Media Source
  Extensions or Web Audio could close the gap later, at a cost in
  complexity.
- **Background tabs and phones.** Browsers throttle timers in background
  tabs (less so while audio plays), which can delay the 1 Hz
  `RendererStatus`. Mobile browsers may suspend a page entirely when it goes
  to the background, which drops the connection. `RemoteEngine` must
  tolerate late status reports (the position is interpolated, §4.2), and a
  dropped renderer falls back to local output as for any other renderer.
- **Formats vary by browser.** Safari's support for Ogg and Opus in
  particular differs from Chrome's and Firefox's. Capabilities come from
  `canPlayType()` at runtime, never from a fixed list per browser.

### Security

Browsers change the threat model. Any page open in a browser on the LAN can
open `ws://192.168.1.10:5500`: WebSockets aren't bound by the same-origin
policy, and the connection comes from a private address, so
`only_non_public_ip` lets it through. So:

- **Check Origin.** Accept WebSocket upgrades only from Clementine's own
  origin.
- **Check Host.** Accept only Host headers that name one of Clementine's own
  addresses or host names, to block DNS rebinding.
- **Rate-limit the auth code.** A script could otherwise try every five-digit
  code in minutes. Limit failed attempts per address, with an increasing
  delay.
- **Keep tokens out of URLs.** The auth code is exchanged for the session
  token from §9. It is sent in the first WebSocket message and kept in
  `sessionStorage`. Media URLs carry it by design (§5), so they must not be
  logged, and the page sets `Referrer-Policy: no-referrer`.
- The same checks also protect the media endpoints, because a malicious page
  could otherwise probe them too.

### HTTPS

The protocol works over plain HTTP. What's missing without HTTPS and a
valid certificate is a browser that treats the page like an app.

- **As a controller,** a page served by Clementine over HTTP works: the
  WebSocket, playlists and controls are all fine. The cost is that pairing
  secrets and session tokens cross the LAN in cleartext.
- **As a renderer,** the page is held back:
  - Browsers only let a page install as an app, or run a service worker,
    over HTTPS (or on localhost). Without them, playback depends on a
    browser tab staying open.
  - Other APIs, such as Wake Lock and parts of Web Crypto, are also limited
    to HTTPS. The rules differ by browser, so assume HTTP is missing things.
  - A self-signed certificate doesn't help. Every device gets a full-page
    warning, browsers remember the exception differently, and a page with an
    invalid certificate still can't install. A native app can pin a
    fingerprint from a QR code; a browser can't.
- **A page hosted anywhere else needs TLS on Clementine.** An HTTPS page
  can't open a plain `ws://` or `http://` connection to a LAN address
  (mixed content), and Chrome now also asks the user's permission before a
  public site connects to a private address.

So the choice is between HTTP served by Clementine, with those limits, and a
valid certificate for a name that points at the Clementine machine. There
are three ways to get there.

#### Option 1: plain HTTP from Clementine

No setup, and the baseline everything else falls back to. The UX work is in
pairing and in being honest about the limits:

- **Pairing instead of the five-digit code.** The settings page shows a QR
  code carrying Clementine's URL and a one-time pairing secret. The browser
  trades the secret for a long-lived device token, and the settings page
  lists paired devices with a Revoke button. This beats the auth code under
  either option, though over HTTP the token can be sniffed on the LAN.
- **Setting expectations.** The page says plainly: "Keep this tab open. On
  phones, playback may stop when the screen locks." A suspended page drops
  its WebSocket and never hears about the next track, which makes open
  question 3 (a renderer that queues or pulls the next track itself) more
  pressing for web renderers.

#### Option 2: bring your own certificate

Costs the project nothing to run, and it's what power users will want.

- **Certificate and key settings.** Clementine serves HTTPS on the same
  port: a TLS ClientHello starts with `0x16`, which the protocol sniffer can
  recognise (§5.0). It watches the files and reloads them when they're
  renewed.
- **Tailscale** is the lowest-effort way to a real certificate, with no
  domain of your own; see the steps below.
- **A reverse proxy** (Caddy, nginx) in front of Clementine also works, with
  the adjustments listed under "What Clementine needs behind a proxy".
- **Your own domain with ACME** needs DNS-01 validation through the DNS
  provider's API, because HTTP-01 can't reach a LAN machine. That's fine for
  the few who want it, and too much to build a UI around.

#### Option 3: a hosted service, like Plex's

Plex gives every server a valid certificate with no user effort:

1. Plex's DNS answers names that encode an IP address:
   `192-168-1-10.<server id>.plex.direct` resolves to `192.168.1.10`.
2. Each server gets a wildcard certificate for `*.<server id>.plex.direct`.
3. The hosted app (app.plex.tv, on HTTPS) connects to
   `https://192-168-1-10.<server id>.plex.direct:32400`. The certificate is
   valid and nothing is blocked as mixed content.
4. Accounts tie it together: the app lists the servers you've signed in on.

A Clementine equivalent (say `clementine.direct`) would need:

- **A DNS server** answering IP-encoded names, on a domain listed on the
  Public Suffix List. Without the listing, Let's Encrypt's limit of about 50
  certificates per registered domain per week caps the whole project.
- **A certificate broker.** Clementine makes its key and certificate request
  locally, and the service completes the DNS-01 challenge for that
  install's subdomain. The service never sees the private key.
- **Proof of installation,** so the broker only issues a subdomain's
  certificate to the install that owns it. That means registering installs,
  which is close to accounts.
- **Renewal while Clementine runs.** Certificate lifetimes are shrinking
  (CA/Browser Forum rules bring the maximum down to 47 days by 2029), so an
  install left closed for a few weeks comes back with an expired
  certificate and must fall back to Option 1 cleanly.
- **Discovery.** Without accounts, the pairing QR code can carry the
  `clementine.direct` URL. That works, but less smoothly than signing in.

The costs:

- **Running it indefinitely.** If the service or domain lapses, every
  install's web remote breaks in a way users can't fix.
- **Privacy.** The service learns every install's LAN address, public
  address and when it runs, so it needs a published policy.
- **Some networks block it.** Routers and DNS filters with DNS-rebinding
  protection refuse public names that resolve to private addresses. Plex
  documents this as a common support problem; those users silently get
  Option 1.
- **Abuse.** Automatically issued certificates on a public domain attract
  misuse, which could get the domain flagged.
- **Remote access is separate.** Plex's relay for playback away from home
  carries user traffic and is a much bigger service. Tailscale or WireGuard
  cover that instead.

#### Recommendation

1. **Build Option 1 properly first:** pairing tokens, a revocable device
   list, the checks under Security above, and honest messaging about mobile
   background playback.
2. **Add Option 2 next:** certificate and key settings, TLS on the shared
   port, and the Tailscale recipe below. That serves the people who most
   want HTTPS at no ongoing cost.
3. **Keep Option 3 possible without committing to it:** keep the
   certificate source abstract (files now, a broker later), make the web app
   work whether Clementine serves it or it's hosted with an allowlisted
   Origin, and make pairing carry a URL rather than assume `host:port`.
   Whether the project wants to run public infrastructure is a decision for
   the maintainers, not a technical one.

#### Tailscale, step by step

Both the Clementine computer and the phone or laptop running the browser
need to be on the same tailnet. Traffic between them is already encrypted
by WireGuard; the certificate is there so the browser treats the page as
secure. A bonus: it also works away from home.

1. Install Tailscale on the Clementine computer and on each device that
   will open the web remote, and sign in to the same tailnet on all of
   them.
2. In the Tailscale admin console, open **DNS**, make sure **MagicDNS** is
   on, and under **HTTPS Certificates** choose **Enable HTTPS**. The page
   warns that machine names are published in the public Certificate
   Transparency logs. Rename the Clementine machine first if its name
   reveals anything you'd rather not publish.
3. On the Clementine computer, publish the remote's port over HTTPS:

   ```sh
   tailscale serve --bg --https=443 localhost:5500
   tailscale serve status    # shows https://<machine>.<tailnet>.ts.net
   ```

   On Linux, if it refuses for lack of permission, allow your user to
   manage Tailscale once with `sudo tailscale set --operator=$USER`.
4. Open `https://<machine>.<tailnet>.ts.net` on the phone and pair as
   usual.

To stop: `tailscale serve --https=443 localhost:5500 off`.

The alternative to step 3 is to let Clementine terminate TLS itself
(Option 2): `tailscale cert <machine>.<tailnet>.ts.net` writes a Let's
Encrypt certificate and key to files that Clementine's settings point at.
Tailscale's docs note that you then have to renew them yourself before they
expire after 90 days, for example with a timer that re-runs the command,
and Clementine reloads them when they change. `tailscale serve` renews
automatically, so it's the better default.

#### What Clementine needs behind a proxy

`tailscale serve`, Caddy and nginx all put a proxy between the browser and
Clementine, which changes a few assumptions:

- **Every client appears to come from localhost.** Clementine sees the
  proxy's address, which on a dual-stack socket is `::ffff:127.0.0.1`
  (#7524 makes the private-network check accept it). The network check then
  says nothing about the real client, so access control rests on pairing
  and, for Tailscale, on the tailnet's access rules. The media URL peer
  check (§9) is weaker for the same reason, which is acceptable when the
  tokens are unguessable and only sent over TLS.
- **Media URLs must use the name the client used.** The prototype builds
  them from the control socket's local address, which behind a proxy is
  `127.0.0.1`. They should come from the Host of the WebSocket request, or
  from a configured external URL.
- **Host and Origin checks must allow the public name,** such as the
  `ts.net` name, as well as Clementine's own addresses.
- **WebSockets have to pass through the proxy.** Caddy and nginx support
  this; confirm it for `tailscale serve` before relying on it.
- **Only HTTP goes through an HTTP proxy.** The native Android remote keeps
  speaking the protobuf protocol on the port directly. Over Tailscale it can
  connect to the machine's tailnet address, which is already encrypted.

### Existing messages that don't suit a browser

The web client can reuse most of the protocol as it is, but not all of it:

- **`GET_LIBRARY`** sends the whole library as a SQLite database in chunks.
  A browser could read it only by loading SQLite compiled to WebAssembly,
  and a large library is a big download to a phone. A web client would want
  paged library queries (artists, albums, search) instead.
- **`DOWNLOAD_SONGS`** sends files as `SONG_FILE_CHUNK` messages, which a
  browser would have to reassemble in memory. Downloads should instead be
  plain HTTP responses from the media endpoint, which the browser saves
  natively.
- **Album art** arrives as bytes inside `SongMetadata.art`. That works, but
  an image URL on the same server would let the browser cache art.

Those are the pieces to design before a web client is practical. The
transport itself is the small part.
