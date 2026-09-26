# Design: streaming to remote devices through the network remote

Status: proposal
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
- **Restreaming**: Clementine resolves, decodes or re-encodes the source and
  serves the result, because the device couldn't play the original. That
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
                   ───┼─ HTTP :5500 GET /s/<token>/<item> ► Direct | Proxy | Transcode    │
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
  scope unless it is transcoding (§5.3), where it can feed buffers.
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
| `Error` / `InvalidMediaRequested` | `RENDERER_ERROR`, which makes `Player` skip exactly as it does for local errors |
| `ValidMediaRequested`        | first `RENDERER_STATUS` in state Playing for that item id |

Crossfade and fade-out need two decoders mixed in one place, which a renderer
doesn't have. `RemoteEngine` turns them off (`is_autocrossfade_enabled()` and
friends return false) unless it is using live-mix mode (§5.4).

Losing the renderer (socket closed, or keep-alive timeout reusing
`OutgoingDataCreator`'s keep-alive) makes the router fall back to local
output, **paused** at the last reported position. Audio that suddenly starts
on the computer would be a bad surprise. `OUTPUT_CHANGED` tells controllers
what happened.

### 4.3 `StreamPlanner`: choosing stream or restream

`StreamPlanner` is a pure function, so it is easy to unit-test. It decides how
to deliver each track:

```
Plan StreamPlanner::Plan(const MediaPlaybackRequest& req, const Song& song,
                         const RendererCaps& caps, const StreamSettings& s);

struct Plan {
  enum Mode { Direct, Proxy, Transcode };
  Mode mode;
  QString mime_type;          // what the renderer will receive
  TranscoderPreset preset;    // Transcode only
  qint64 length_nanosec;      // known length, used for seek bars and ?t=
};
```

The first rule that matches wins:

| # | Condition | Mode |
|---|-----------|------|
| 1 | The user wants DSP applied remotely (`apply_dsp`) and EQ or ReplayGain is on | Transcode |
| 2 | `song.has_cue()`, or a begin/end marker is set (a track cut from a larger file) | Transcode |
| 3 | `song.is_cdda()`, or the media URL scheme isn't file/http(s) | Transcode |
| 4 | The codec isn't in `caps.codecs`, or the sample rate or channel count is above what the renderer supports | Transcode |
| 5 | `song.IsFileLossless()` and `s.transcode_lossless` (the same setting downloads use) | Transcode |
| 6 | The file's bitrate is above `s.max_bitrate_kbps` (if set) | Transcode |
| 7 | Local file | **Direct** |
| 8 | http(s) URL that has headers (`Authorization`) or that the renderer said it can't reach | **Proxy** |
| 9 | Other http(s) URL (for example an internet radio stream) | Proxy by default. Handing the renderer the raw URL is an option (`s.allow_raw_urls`), but it reveals the URL and loses control of the stream |

"Stream" in the request means Direct. "Restream" means Proxy (the same bytes,
fetched by Clementine with Clementine's credentials) or Transcode (decoded by
Clementine and encoded again).

## 5. Delivery modes

All three are served by `MediaHttpServer`, a small HTTP/1.1 handler that
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

### 5.2 Proxy: restreaming bytes

- When a request arrives, `MediaHttpServer` opens an upstream request with
  `QNetworkAccessManager` (so the app's proxy settings apply) and adds
  `req.headers_` (for example `Authorization`, the header
  `UrlHandler::LoadResult::auth_header_` provides). It forwards the renderer's
  `Range` header and relays the status, `Content-Type`, `Content-Length` and
  `Content-Range`.
- It uses back-pressure: it reads from the upstream reply only while the
  socket's `bytesToWrite()` is below a high-water mark. The reply's
  `readBufferSize` is capped.
- A short-lived signed URL, such as the one Subsonic's or Plex's `UrlHandler`
  gives, is resolved again with `Player`'s `UrlHandler` if the upstream
  returns 401 or 403 in the middle of a stream, then the request is retried
  once.
- Live radio (no `Content-Length`) is relayed as chunked transfer. Metadata
  that arrives in the stream (`MetaData` signals from `GstEngine`) isn't
  available in Proxy mode. §10 covers this.

### 5.3 Transcode: restreaming decoded audio

A new `GstStreamPipeline : GstPipelineBase` is built for each request:

```
uridecodebin(uri=req.MediaUrl, extra-headers=req.headers_)
  ! audioconvert ! [rgvolume ! rglimiter]  ! [equalizer-nbands ! audiopanorama]
  ! audioresample ! capsfilter(rate/channels <= caps)
  ! <preset encoder> ! <preset muxer> ! appsink
```

- The encoder and muxer come from the **existing `TranscoderPreset`s**, the
  ones already offered for downloads (Ogg Vorbis/Opus, MP3, AAC/M4A, FLAC),
  so there are no new format lists to maintain. The default is Opus in Ogg at
  128 kbps when the renderer lists it, otherwise MP3 at 192 kbps.
- The ReplayGain, EQ and balance elements are made with the same helper
  `GstEnginePipeline::InitAudioBin` uses. That code moves into a shared
  function so local and remote playback sound the same.
- Begin and end markers (cue tracks) become a segment seek on the decoder,
  the same way `GstEnginePipeline` handles them.
- Transcoded output can't be byte-ranged, so seeking uses `?t=<ms>`. The
  renderer gets `RENDER_SEEK{position, url}` with a new URL, and the server
  starts a new pipeline that seeks to `t` before going to PLAYING. The
  response has no `Content-Length` and `Accept-Ranges: none`. Length comes
  from `Song::length_nanosec` in `RENDER_LOAD`, so the renderer's seek bar is
  still correct.
- CPU use is bounded: at most one active transcode per renderer, plus one to
  preload the next track. The pipeline pauses when `appsink` gets ahead of
  the socket by more than N seconds of audio.
- The pipeline can optionally tee into `BufferConsumer`, so the analyzer and
  moodbar in Clementine's window keep moving while it plays remotely.

### 5.4 Live mix (later phase)

This mode keeps crossfades, fade-outs and one continuous stream across
tracks. The engine's audio bin ends in `encoder ! muxer ! appsink` instead of
`audiosink`, and every renderer gets one unending HTTP stream (like Icecast).
It uses the same `set_output_device` path in `GstEnginePipeline`, plus an
output option for "remote mixed stream".

Costs: 2–5 s of latency between a control action and what the user hears,
position that has to be adjusted for the renderer's buffer, and no native
seeking on the renderer. It is worth having for "party mode", but it
shouldn't be the default. Direct, Proxy and Transcode are Phase 1 and 2.

## 6. Protocol changes

All changes add to the existing messages, so existing clients and servers are
unaffected. `Message.version` goes from 21 to 22. Clients check
`ResponseClementineInfo.supports_rendering` before using any of it, and the
server only offers rendering to clients whose `RequestConnect` includes
`renderer`.

```proto
enum MsgType {
  // ...existing values...

  // Controller/renderer -> server
  REGISTER_RENDERER     = 300;
  UNREGISTER_RENDERER   = 301;
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

  // Server -> all clients
  OUTPUTS               = 340;
  OUTPUT_CHANGED        = 341;
}

message RendererCapabilities {
  optional string renderer_id = 1;    // stable per install (UUID), used for "remember"
  optional string display_name = 2;   // "Pixel 9", "Kitchen tablet"
  repeated string mime_types = 3;     // "audio/flac", "audio/ogg; codecs=opus", "audio/mpeg"
  optional int32 max_sample_rate = 4;
  optional int32 max_channels = 5;
  optional bool gapless = 6;          // can queue RENDER_PRELOAD
  optional bool http_range = 7;       // honours Range (Direct seeking)
  optional int32 max_bitrate_kbps = 8;// e.g. "on mobile data"
}

message RequestConnect {
  // ...existing fields 1..3...
  optional RendererCapabilities renderer = 4;  // present => wants to be a renderer
}

message ResponseClementineInfo {
  // ...existing fields 1..4...
  optional bool supports_rendering = 5;  // server has streaming enabled
}

message Output {
  optional string output_id = 1;      // "local" or renderer_id
  optional string display_name = 2;
  optional bool active = 3;
}
message ResponseOutputs { repeated Output outputs = 1; }
message RequestSetOutput { optional string output_id = 1; }

enum StreamMode { STREAM_DIRECT = 0; STREAM_PROXY = 1; STREAM_TRANSCODE = 2; }

message RenderItem {
  optional int32 item_id = 1;
  optional string url = 2;            // http://host:5500/s/<token>/<item_id>
  optional string mime_type = 3;
  optional StreamMode mode = 4;
  optional int64 length_ms = 5;
  optional bool seek_by_url = 6;      // true for Transcode: use RENDER_SEEK.url
  optional SongMetadata song = 7;     // for lock screen / notification UI
}

message RequestRenderLoad {
  optional RenderItem item = 1;
  optional int64 start_ms = 2;
  optional bool play = 3;
}
message RequestRenderSeek {
  optional int32 item_id = 1;
  optional int64 position_ms = 2;
  optional string url = 3;            // set when item.seek_by_url
}
message RequestRenderVolume { optional int32 volume = 1; }  // 0..100

message RendererStatus {
  optional int32 item_id = 1;
  optional EngineState state = 2;
  optional int64 position_ms = 3;
  optional int32 buffered_percent = 4;
}
message RendererError {
  optional int32 item_id = 1;
  optional string message = 2;
  optional bool fatal = 3;            // false => track-level, skip; true => drop renderer
}

message Message {
  // ...existing fields...
  optional RendererCapabilities renderer_capabilities = 60;
  optional ResponseOutputs response_outputs = 61;
  optional RequestSetOutput request_set_output = 62;
  optional RequestRenderLoad request_render_load = 63;
  optional RenderItem request_render_preload = 64;
  optional RequestRenderSeek request_render_seek = 65;
  optional RequestRenderVolume request_render_volume = 66;
  optional RendererStatus renderer_status = 67;
  optional RendererError renderer_error = 68;
  optional int32 renderer_track_ended_item_id = 69;
}
```

Notes

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
  any changes.

## 7. Code layout

```
src/engines/enginerouter.{h,cpp}           EngineRouter (Engine::Base)
src/engines/gstaudiobinhelper.{h,cpp}      RG/EQ/balance elements shared by GstEnginePipeline and GstStreamPipeline
src/networkremote/streaming/
    remoteengine.{h,cpp}                   RemoteEngine (Engine::Base)
    rendererregistry.{h,cpp}               renderer_id -> RemoteEngine; owns OUTPUTS/OUTPUT_CHANGED
    streamplanner.{h,cpp}                  pure planning logic
    protocolsniffer.{h,cpp}                first-byte routing of accepted sockets (§5.0)
    mediahttpserver.{h,cpp}                HTTP request parsing, token check, dispatch
    directresponder.{h,cpp}                file + Range
    proxyresponder.{h,cpp}                 QNetworkAccessManager relay
    transcoderesponder.{h,cpp}             owns a GstStreamPipeline, appsink -> socket
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
- `OutgoingDataCreator::SendClementineInfo`: set `supports_rendering`.
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
  - *Apply equalizer and ReplayGain on remote devices* (forces Transcode)
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
- **Credentials stay in Clementine.** In Proxy and Transcode mode the renderer
  gets Clementine's URL, never the upstream URL or its `Authorization`
  header. Raw-URL handoff (`allow_raw_urls`) only applies to URLs without
  headers, and it is off by default.
- **No path exposure.** Files are chosen by `item_id`, never by path. That
  sidesteps the whole class of traversal bugs `files_root_folder` handling
  has to guard against.
- **Resource limits.** A cap on concurrent HTTP responses per session (4), on
  transcodes per renderer (2) and on header size (8 KiB). Idle sockets time
  out after 30 s.
- **Known weakness, unchanged.** The remote's auth code is a short number
  sent in plaintext. This design doesn't make that worse. Tokens are only
  handed out on authenticated connections. TLS could be added later on the
  same port, since its first byte is distinguishable too (§5.0).

## 10. Edge cases

| Case | Behaviour |
|------|-----------|
| Renderer disconnects mid-track | Fall back to local, paused at the last position. Send `OUTPUT_CHANGED` |
| Renderer reports a track-level error | `Error` + `InvalidMediaRequested`. `Player` skips to the next track, as it does locally |
| The same track fails Direct because the codec was advertised but doesn't actually decode | Renderer sends `RENDERER_ERROR{fatal=false}`. `RemoteEngine` tries the same item once more, forcing Transcode, before reporting an error |
| Stream metadata changes (radio) in Proxy mode | Not available. `RemoteEngine` switches radio to Transcode when "show stream titles" matters (setting TBD), because the decoder then emits tags into `MetaData` |
| Clementine quits while playing remotely | `RENDER_STOP`, then the usual `DISCONNECT{Server_Shutdown}` |
| Two controllers choose different outputs | Last write wins. Everyone gets `OUTPUT_CHANGED` |
| Hand-off during Transcode | New `RENDER_LOAD` with `start_ms`. The server makes a fresh pipeline seeked to that point |
| Stop after current, repeat, shuffle, queue | Unchanged. They are `Player`/`PlaylistSequence` logic above the engine |
| Scrobbling / play counts | Unchanged. They use `Player`'s position and state signals, which the router drives |

## 11. Phasing

1. **Phase 1: streaming local files.** Protocol v22, `EngineRouter`,
   `RemoteEngine`, `RendererRegistry`, `MediaHttpServer` with Direct only,
   `StreamPlanner` rules 7 and 4 (unsupported tracks are skipped with a clear
   error), output picker, settings. The Android remote gets "Play on this
   phone". This version is already useful for most local libraries.
2. **Phase 2: restreaming.** Proxy mode (internet services, radio) and
   Transcode mode (cue, CDDA, unsupported codecs, lossless and bitrate caps,
   optional DSP), `?t=` seeking, sharing the audio-bin helper.
3. **Phase 3: more renderers and live mix.** Because delivery is plain HTTP,
   UPnP/DLNA MediaRenderers (`AVTransport.SetAVTransportURI`) and Chromecast
   (Default Media Receiver `LOAD`) are just more `RemoteEngine` backends
   behind an `IRendererTransport` interface. They are discovered with SSDP or
   mDNS rather than registering themselves. Live-mix mode (§5.4) comes here
   too.

## 12. Testing

- `streamplanner_test.cpp`: a table-driven test of every rule in §4.3. It is
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
- Extend `networkremote_test.cpp` to check that `supports_rendering` is only
  advertised when enabled.
- `protocolsniffer_test.cpp`: first byte `0x00`–`0x08` goes to the remote,
  `A`–`Z` to HTTP only when streaming is on, anything else is closed; a
  first byte that arrives in a separate TCP segment from the rest; the
  10 s silence timeout.
- Transcode: a GStreamer test that turns a short WAV fixture into Ogg/Opus
  through `GstStreamPipeline` and checks the caps and approximate duration,
  including a `?t=` start.

## 13. Open questions

1. Should Clementine's volume slider set the renderer's device volume, or a
   software gain inside Transcode? The proposal is the renderer's volume
   (`RENDER_SET_VOLUME`), because Direct mode can't apply gain.
2. Should the choice of output be per playlist ("the kitchen playlist always
   plays in the kitchen")? It's out of scope, but `EngineRouter::SetOutput`
   should be callable from `PlaylistManager` later.
3. Should a renderer be able to *pull* the next track itself (a playlist
   cursor on the renderer) to survive short losses of the control connection
   on mobile? This design keeps the renderer passive. That is simpler and
   keeps one source of truth, at the cost of a gap if the control connection
   is down exactly when a track ends. `RENDER_PRELOAD` reduces the risk.
