# Apple senders' AirPlay 2 sessions, decoded

What Apple's senders sent to an AirPlay 2 receiver on 2026-11-01. The
receiver was shairport-sync 5.6.2, built with AirPlay 2 support and run with
nqptp 1.2.9 at its most verbose, so it logged each request decrypted, with
the headers and plists below. POST /command bodies come from its metadata
pipe, which has them whole.

Personal values are replaced with placeholders, the same placeholder for the
same value throughout: `<uuid-N>`, `<ip-N>`, `<mac-N>`, `<url-N>`,
`<title-N>` and so on. Artwork is replaced with its size.

## A Mac's system audio

A Mac14,2 (macOS 27.0.1, AirPlay 980.77.5), with the receiver chosen as
its sound output in Control Centre, playing music, then paused and resumed.

### What it shows

- It used realtime audio (stream type 96) of ALAC at 44.1 kHz/16-bit
  (`ct` 2, `audioFormat` 0x40000, 352 frames a packet), with PTP timing:
  the same stream as Clementine's, apart from the format and the timing.
  So no SETRATEANCHORTIME and no FLUSHBUFFERED; a FLUSH with RTP-Info
  before the audio.
- Its session SETUP has the iPhone's fields: PTP timing, `groupUUID`,
  `sessionCorrelationUUID`, `asyncPTPClockConfig`.
- It started as the iPhone did: empty now-playing info, no supported
  commands, then seven: 0 to 5 (play, pause, play/pause, stop, next,
  previous) and 24 (seek, not by scrubbing).
- What's playing went in POST /command `updateMRNowPlayingInfo` as a
  `replace`, with the cover and its MIME type, the duration, elapsed time
  0, rate 1 and a timestamp, a `ContentItemIdentifier`, and `MediaType`
  `kMRMediaRemoteNowPlayingInfoTypeAudio`. There was no SET_PARAMETER
  of DMAP, artwork or progress; one SET_PARAMETER of the volume.
- The pause was a `replace` with rate 0 and the elapsed time, under a new
  `ContentItemIdentifier`, then an `update` with the cover. Nothing more was
  sent before the TEARDOWN.

### Requests, in order

#### 1. GET /info

```
X-Apple-ProtocolVersion: 1
Content-Length: 70
Content-Type: application/x-apple-binary-plist
CSeq: 0
User-Agent: AirPlay/980.77.5
```

```
{
  "qualifier" => [
    0 => "txtAirPlay"
  ]
}
```

#### 2. POST /pair-setup

```
X-Apple-HKP: 4
X-Apple-Client-Name: <sender name>
X-Apple-Client-ID: <uuid-1>
Content-Length: 9
Content-Type: application/x-apple-binary-plist
CSeq: 1
User-Agent: AirPlay/980.77.5
```

#### 3. POST /pair-setup

```
X-Apple-HKP: 4
X-Apple-Client-Name: <sender name>
X-Apple-Client-ID: <uuid-1>
Content-Length: 457
Content-Type: application/x-apple-binary-plist
CSeq: 2
User-Agent: AirPlay/980.77.5
```

#### 4. POST /fp-setup

```
X-Apple-ET: 32
Content-Length: 16
Content-Type: application/octet-stream
CSeq: 3
User-Agent: AirPlay/980.77.5
```

#### 5. POST /fp-setup

```
X-Apple-ET: 32
Content-Length: 164
Content-Type: application/octet-stream
CSeq: 4
User-Agent: AirPlay/980.77.5
```

#### 6. SETUP <session URI>

```
X-Apple-ProtocolVersion: 1
Content-Length: 856
Content-Type: application/x-apple-binary-plist
CSeq: 5
User-Agent: AirPlay/980.77.5
```

```
{
  "osVersion" => "27.0.1"
  "timingPeerList" => [
    0 => {
      "ID" => "<uuid-1>"
      "DeviceType" => 0
      "SupportsClockPortMatchingOverride" => True
    }
  ]
  "isMultiSelectAirPlay" => True
  "combinedGetInfoWithControlSetup" => True
  "name" => <name-1>
  "sessionCorrelationUUID" => "<uuid-1>"
  "groupUUID" => "<uuid-2>"
  "deviceID" => <mac-1>
  "timingProtocol" => "PTP"
  "timingPeerInfo" => {
    "ID" => "<uuid-1>"
    "DeviceType" => 0
    "SupportsClockPortMatchingOverride" => True
  }
  "sourceVersion" => "980.77.5"
  "senderSupportsRelay" => True
  "groupContainsGroupLeader" => False
  "statsCollectionEnabled" => False
  "asyncPTPClockConfig" => True
  "model" => "Mac14,2"
  "macAddress" => <mac-2>
  "sessionUUID" => "<uuid-3>"
  "osBuildVersion" => "26A434"
  "updateSessionRequest" => False
  "osName" => "macOS"
}
```

#### 7. GET /info

```
X-Apple-ProtocolVersion: 1
CSeq: 6
User-Agent: AirPlay/980.77.5
```

```
{
  "playbackCapabilities" => {
    "supportsInterstitials" => False
    "supportsFPSSecureStop" => False
    "supportsUIForAudioOnlyContent" => False
  }
  "canRecordScreenStream" => False
  "keepAliveSendStatsAsBody" => False
  "protocolVersion" => "1.1"
  "screenDemoMode" => False
  "psi" => <uuid-4>
  "featuresEx" => "AEocQEDDBQA"
  "features" => 1622055604472320
  "statusFlags" => 4
  "deviceID" => <mac-3>
  "pi" => <uuid-5>
  "name" => "zem test"
  "model" => "ShairportSync"
  "pk" => <32-byte public key>
  "senderAddress" => "<ip-1>:51082"
  "initialVolume" => -24.0
  "vv" => 0
  "volumeControlType" => 3
  "supportedFormats" => {
    "audioStream" => 262144
    "bufferStream" => 1649282383872
  }
  "txtAirPlay" => <410 bytes: the receiver's TXT record>
}
```

#### 8. RECORD <session URI>

```
CSeq: 7
User-Agent: AirPlay/980.77.5
```

#### 9. SETPEERS <session URI>

```
Content-Length: 131
Content-Type: /peer-list-changed
CSeq: 8
User-Agent: AirPlay/980.77.5
```

```
[
  0 => "<ip-1>"
  1 => "<ip-2>"
  2 => "<ip-3>"
]
```

#### 10. POST /command

```
Content-Length: 128
Content-Type: application/x-apple-binary-plist
CSeq: 9
User-Agent: AirPlay/980.77.5
```

Body: a `updateMRNowPlayingInfo` command, shown in full below.

#### 11. POST /command

```
Content-Length: 128
Content-Type: application/x-apple-binary-plist
CSeq: 10
User-Agent: AirPlay/980.77.5
```

Body: a `updateMRSupportedCommands` command, shown in full below.

#### 12. POST

```
CSeq: 11
User-Agent: AirPlay/980.77.5
```

Empty; shairport-sync didn't log its path.

#### 13. POST /command

```
Content-Length: 1048
Content-Type: application/x-apple-binary-plist
CSeq: 12
User-Agent: AirPlay/980.77.5
```

Body: a `updateMRSupportedCommands` command, shown in full below.

#### 14. SETUP <session URI>

```
X-Apple-ProtocolVersion: 1
Content-Length: 350
Content-Type: application/x-apple-binary-plist
CSeq: 13
User-Agent: AirPlay/980.77.5
```

```
{
  "streams" => [
    0 => {
      "audioFormat" => 262144
      "supportsDynamicStreamID" => True
      "streamConnectionID" => <stream-id-1>
      "ct" => 2
      "audioMode" => "default"
      "spf" => 352
      "controlPort" => 51686
      "latencyMax" => 88200
      "isMedia" => True
      "type" => 96
      "sr" => 44100
      "shk" => <32-byte audio key>
      "latencyMin" => 11025
    }
  ]
}
```

#### 15. SET_PARAMETER <session URI>

```
Content-Length: 20
Content-Type: text/parameters
CSeq: 14
User-Agent: AirPlay/980.77.5
```

#### 16. FLUSH <session URI>

```
RTP-Info: seq=19602;rtptime=3831021328
CSeq: 15
User-Agent: AirPlay/980.77.5
```

#### 17. POST /feedback

```
CSeq: 16
User-Agent: AirPlay/980.77.5
```

```
{
  "streams" => [
    0 => {
      "type" => 96
      "sr" => 44100.0
    }
  ]
}
```

#### 18. POST /command

```
Content-Length: 213253
Content-Type: application/x-apple-binary-plist
CSeq: 17
User-Agent: AirPlay/980.77.5
```

Body: a `updateMRNowPlayingInfo` command, with the cover, shown in full
below.

#### 19. POST /feedback

```
CSeq: 19
User-Agent: AirPlay/980.77.5
```

```
{
  "streams" => [
    0 => {
      "type" => 96
      "sr" => 44100.0
    }
  ]
}
```

#### 20. POST /feedback

```
CSeq: 20
User-Agent: AirPlay/980.77.5
```

```
{
  "streams" => [
    0 => {
      "type" => 96
      "sr" => 44100.0
    }
  ]
}
```

#### 21. POST /feedback

```
CSeq: 21
User-Agent: AirPlay/980.77.5
```

```
{
  "streams" => [
    0 => {
      "type" => 96
      "sr" => 44100.0
    }
  ]
}
```

#### 22. POST /feedback

```
CSeq: 22
User-Agent: AirPlay/980.77.5
```

```
{
  "streams" => [
    0 => {
      "type" => 96
      "sr" => 44100.0
    }
  ]
}
```

#### 23. POST /feedback

```
CSeq: 23
User-Agent: AirPlay/980.77.5
```

```
{
  "streams" => [
    0 => {
      "type" => 96
      "sr" => 44100.0
    }
  ]
}
```

#### 24. POST /feedback

```
CSeq: 24
User-Agent: AirPlay/980.77.5
```

```
{
  "streams" => [
    0 => {
      "type" => 96
      "sr" => 44100.0
    }
  ]
}
```

#### 25. POST /feedback

```
CSeq: 25
User-Agent: AirPlay/980.77.5
```

```
{
  "streams" => [
    0 => {
      "type" => 96
      "sr" => 44100.0
    }
  ]
}
```

#### 26. POST /feedback

```
CSeq: 26
User-Agent: AirPlay/980.77.5
```

```
{
  "streams" => [
    0 => {
      "type" => 96
      "sr" => 44100.0
    }
  ]
}
```

#### 27. POST /command

```
Content-Length: 1023
Content-Type: application/x-apple-binary-plist
CSeq: 27
User-Agent: AirPlay/980.77.5
```

Body: a `updateMRNowPlayingInfo` command, shown in full below.

#### 28. POST /feedback

```
Content-Length: 213252
Content-Type: application/x-apple-binary-plist
CSeq: 28
User-Agent: AirPlay/980.77.5
```

#### 29. POST /feedback

```
CSeq: 30
User-Agent: AirPlay/980.77.5
```

```
{
  "streams" => [
    0 => {
      "type" => 96
      "sr" => 44100.0
    }
  ]
}
```

#### 30. POST /feedback

```
CSeq: 31
User-Agent: AirPlay/980.77.5
```

```
{
  "streams" => [
    0 => {
      "type" => 96
      "sr" => 44100.0
    }
  ]
}
```

#### 31. POST /feedback

```
CSeq: 32
User-Agent: AirPlay/980.77.5
```

```
{
  "streams" => [
    0 => {
      "type" => 96
      "sr" => 44100.0
    }
  ]
}
```

#### 32. POST /feedback

```
CSeq: 33
User-Agent: AirPlay/980.77.5
```

```
{
  "streams" => [
    0 => {
      "type" => 96
      "sr" => 44100.0
    }
  ]
}
```

#### 33. POST /feedback

```
CSeq: 34
User-Agent: AirPlay/980.77.5
```

```
{
  "streams" => [
    0 => {
      "type" => 96
      "sr" => 44100.0
    }
  ]
}
```

#### 34. TEARDOWN <session URI>

```
Content-Length: 42
Content-Type: application/x-apple-binary-plist
CSeq: 35
User-Agent: AirPlay/980.77.5
```

```
{}
```

### POST /command bodies, in order

```
{
  "type" => "updateMRNowPlayingInfo"
  "params" => {
    "type" => "npi-text"
    "params" => {}
    "mergePolicy" => "replace"
  }
}
```

```
{
  "type" => "updateMRSupportedCommands"
  "params" => {
    "mrSupportedCommandsFromSender" => []
  }
}
```

```
{
  "type" => "updateMRSupportedCommands"
  "params" => {
    "mrSupportedCommandsFromSender" => [
      0 => <binary plist> {
        "kCommandInfoOptionsKey" => {
          "kMRMediaRemoteCommandInfoCanBeControlledByScrubbingKey" => False
          "kMRMediaRemoteCommandInfoSupportsReferencePosition" => False
        }
        "kCommandInfoCommandKey" => 24
        "kCommandInfoEnabledKey" => True
      }
      1 => <binary plist> {
        "kCommandInfoCommandKey" => 5
        "kCommandInfoEnabledKey" => True
      }
      2 => <binary plist> {
        "kCommandInfoCommandKey" => 4
        "kCommandInfoEnabledKey" => True
      }
      3 => <binary plist> {
        "kCommandInfoCommandKey" => 3
        "kCommandInfoEnabledKey" => True
      }
      4 => <binary plist> {
        "kCommandInfoCommandKey" => 2
        "kCommandInfoEnabledKey" => True
      }
      5 => <binary plist> {
        "kCommandInfoCommandKey" => 1
        "kCommandInfoEnabledKey" => True
      }
      6 => <binary plist> {
        "kCommandInfoCommandKey" => 0
        "kCommandInfoEnabledKey" => True
      }
    ]
  }
}
```

(shairport-sync: `pbeg`)

```
{
  "type" => "updateMRNowPlayingInfo"
  "params" => {
    "type" => "npi-text"
    "params" => {
      "kMRMediaRemoteNowPlayingInfoDuration" => 207.699
      "kMRMediaRemoteNowPlayingInfoAlbum" => <album-1>
      "kMRMediaRemoteNowPlayingInfoTimestamp" => 2026-11-01T23:42:52.611689Z
      "kMRMediaRemoteNowPlayingInfoStrictMediaType" => "audio"
      "kMRMediaRemoteNowPlayingInfoArtworkMIMEType" => "image/jpeg"
      "kMRMediaRemoteNowPlayingInfoTitle" => <title-1>
      "kMRMediaRemoteNowPlayingInfoArtworkIdentifier" => <artwork-url-1>
      "kMRMediaRemoteNowPlayingInfoElapsedTime" => 0.0
      "kMRMediaRemoteNowPlayingInfoPlaybackRate" => 1.0
      "kMRMediaRemoteNowPlayingInfoMediaType" => "kMRMediaRemoteNowPlayingInfoTypeAudio"
      "kMRMediaRemoteNowPlayingInfoArtworkData" => <600x600 JPEG, 212104 bytes>
      "kMRMediaRemoteNowPlayingInfoArtworkDataHeight" => 600
      "kMRMediaRemoteNowPlayingInfoContentItemIdentifier" => <item-1>
      "kMRMediaRemoteNowPlayingInfoArtist" => <artist-1>
      "kMRMediaRemoteNowPlayingInfoArtworkDataWidth" => 600
    }
    "mergePolicy" => "replace"
  }
}
```

updateMRSupportedCommands again, 7 commands.

```
{
  "type" => "updateMRNowPlayingInfo"
  "params" => {
    "type" => "npi-text"
    "params" => {
      "kMRMediaRemoteNowPlayingInfoDuration" => 207.699
      "kMRMediaRemoteNowPlayingInfoAlbum" => <album-1>
      "kMRMediaRemoteNowPlayingInfoTimestamp" => 2026-11-01T23:43:10.202054Z
      "kMRMediaRemoteNowPlayingInfoStrictMediaType" => "audio"
      "kMRMediaRemoteNowPlayingInfoArtworkMIMEType" => "image/jpeg"
      "kMRMediaRemoteNowPlayingInfoTitle" => <title-1>
      "kMRMediaRemoteNowPlayingInfoArtworkIdentifier" => <artwork-url-1>
      "kMRMediaRemoteNowPlayingInfoElapsedTime" => 14.611814058
      "kMRMediaRemoteNowPlayingInfoPlaybackRate" => 0.0
      "kMRMediaRemoteNowPlayingInfoMediaType" => "kMRMediaRemoteNowPlayingInfoTypeAudio"
      "kMRMediaRemoteNowPlayingInfoArtworkDataHeight" => 768.0
      "kMRMediaRemoteNowPlayingInfoContentItemIdentifier" => <item-2>
      "kMRMediaRemoteNowPlayingInfoArtist" => <artist-1>
      "kMRMediaRemoteNowPlayingInfoArtworkDataWidth" => 768.0
    }
    "mergePolicy" => "replace"
  }
}
```

```
{
  "type" => "updateMRNowPlayingInfo"
  "params" => {
    "type" => "npi-text"
    "params" => {
      "kMRMediaRemoteNowPlayingInfoDuration" => 207.699
      "kMRMediaRemoteNowPlayingInfoAlbum" => <album-1>
      "kMRMediaRemoteNowPlayingInfoTimestamp" => 2026-11-01T23:43:10.202054Z
      "kMRMediaRemoteNowPlayingInfoStrictMediaType" => "audio"
      "kMRMediaRemoteNowPlayingInfoArtworkMIMEType" => "image/jpeg"
      "kMRMediaRemoteNowPlayingInfoTitle" => <title-1>
      "kMRMediaRemoteNowPlayingInfoArtworkIdentifier" => <artwork-url-1>
      "kMRMediaRemoteNowPlayingInfoElapsedTime" => 14.611814058
      "kMRMediaRemoteNowPlayingInfoPlaybackRate" => 0.0
      "kMRMediaRemoteNowPlayingInfoMediaType" => "kMRMediaRemoteNowPlayingInfoTypeAudio"
      "kMRMediaRemoteNowPlayingInfoArtworkData" => <600x600 JPEG, 212104 bytes>
      "kMRMediaRemoteNowPlayingInfoArtworkDataHeight" => 600
      "kMRMediaRemoteNowPlayingInfoContentItemIdentifier" => <item-2>
      "kMRMediaRemoteNowPlayingInfoArtist" => <artist-1>
      "kMRMediaRemoteNowPlayingInfoArtworkDataWidth" => 600
    }
    "mergePolicy" => "update"
  }
}
```

(shairport-sync: `pend`)


## An iPhone, to a Mac, from the Mac's log

The iPhone (iOS 27.0.1) playing from Music to the Mac (macOS 27.0.1), on
2026-11-03, seen from the receiver's side: the Mac's own log, from
`log stream` with the `com.apple.airplay` and `com.apple.amp.mediaremote`
subsystems. Its bodies aren't logged, but the commands are, and what the
receiver does with them; with a receiver that has feature bit 60, which
shairport-sync hasn't, an iPhone sends more than it does to shairport-sync.
Times are seconds; the receiver's MediaRemote player had been left stopped
by the session before.

```
16.274 pair-verify (it had paired before)
16.321 AirPlayReceiverMediaRemoteHelper: Become now playing app.
16.355 timingProtocol = PTP
16.392 Received command: 'updateMRNowPlayingInfo'      (replace)
16.405 Received command: 'updateMRPlaybackState'       → Setting MR playback state to 3 (stopped)
16.430 Received command: 'updateMRSupportedCommands'
16.450 Received command: 'updateMRNowPlayingClient'    → Set MR now playing client to com.apple.airplayd
16.465 Received command: 'updateMRPlaybackState'       → Setting MR playback state to 1 (playing)
16.483 mediaremoted: PlaybackState changed from Stopped to Playing
16.567 Received command: 'updateMRSupportedCommands'
16.587 Received command: 'updateMRNowPlayingClient'
16.698 Received command: 'setMRInfo'
16.733 Setting up stream (type BufferedAudio/103)
17.097 Received command: 'updateMRNowPlayingInfo'
17.121 Received command: 'updateMRPlaybackState'       → 1
38.253 Received command: 'updateMRPlaybackState'       → 2 (paused); mediaremoted: Playing to Paused
38.265 SetRateAndAnchorTime inRate 0
39.938 Received command: 'updateMRPlaybackState'       → 1; mediaremoted: Paused to Playing
```

The body of `updateMRPlaybackState`, as shairport-sync reads it, is
`{type: "updateMRPlaybackState", params: {mrPlaybackState: N}}`; sent so by
Clementine, the Mac logged "Setting MR playback state to N" for it too.

## An iPhone

An iPhone 16 Pro (iOS 27.0.1, AirPlay 980.77.1), first from Music, with a
skip, pauses and resumes and a seek, then from Podcasts.

### What it shows

- Both apps used buffered audio (stream type 103) of AAC-LC at 48 kHz
  (`ct` 4, `audioFormat` 0x800000, 1024 frames a packet), over TCP, with
  PTP timing. Neither used realtime audio.
- The iPhone was the PTP master: its SETRATEANCHORTIME anchors were on its
  own clock (`<sender-ptp-clock-1>`), and shairport-sync told nqptp to
  follow it (`/nqptp T <ip>`, then `B` to play, `P` to pause, `E` to end).
- Playback starts with FLUSHBUFFERED, then SETRATEANCHORTIME with rate 1.
  A pause is SETRATEANCHORTIME with rate 0; a resume, a seek or a new song is
  FLUSHBUFFERED (for a seek or a new song) and SETRATEANCHORTIME with rate 1.
- What's playing goes only in POST /command `updateMRNowPlayingInfo`
  (`npi-text`): a `replace` with everything for each song, including the
  cover as `ArtworkData` with an `ArtworkIdentifier`, a
  `ContentItemIdentifier` naming the item, its duration, elapsed time,
  playback rate and a timestamp; then `update`s, which name the item, with
  the rate and timestamp (and the elapsed time after a seek). There was no
  SET_PARAMETER of DMAP, artwork or progress.
- It also sent `updateMRSupportedCommands`, `POST /audioMode`, and
  `POST /feedback` every two seconds; no DACP-ID or Active-Remote header.

### Requests, in order

#### 1. GET /info

```
X-Apple-ProtocolVersion: 1
Content-Length: 70
Content-Type: application/x-apple-binary-plist
CSeq: 0
User-Agent: AirPlay/980.77.1
```

```
{
  "qualifier" => [
    0 => "txtAirPlay"
  ]
}
```

#### 2. POST /pair-setup

```
X-Apple-AbsoluteTime: 815267912
X-Apple-HKP: 4
X-Apple-Client-Name: <sender name>
X-Apple-Client-ID: <uuid-1>
Content-Length: 9
Content-Type: application/x-apple-binary-plist
CSeq: 1
User-Agent: AirPlay/980.77.1
```

#### 3. POST /pair-setup

```
X-Apple-AbsoluteTime: 815267912
X-Apple-HKP: 4
X-Apple-Client-Name: <sender name>
X-Apple-Client-ID: <uuid-1>
Content-Length: 457
Content-Type: application/x-apple-binary-plist
CSeq: 2
User-Agent: AirPlay/980.77.1
```

#### 4. POST /fp-setup

```
X-Apple-ET: 32
Content-Length: 16
Content-Type: application/octet-stream
CSeq: 3
User-Agent: AirPlay/980.77.1
```

#### 5. POST /fp-setup

```
X-Apple-ET: 32
Content-Length: 164
Content-Type: application/octet-stream
CSeq: 4
User-Agent: AirPlay/980.77.1
```

#### 6. SETUP <session URI>

```
X-Apple-ProtocolVersion: 1
Content-Length: 948
Content-Type: application/x-apple-binary-plist
CSeq: 5
User-Agent: AirPlay/980.77.1
```

```
{
  "osVersion" => "27.0.1"
  "timingPeerList" => [
    0 => {
      "ID" => "<uuid-1>"
      "DeviceType" => 0
      "SupportsClockPortMatchingOverride" => True
    }
  ]
  "internalBuild" => False
  "isMultiSelectAirPlay" => True
  "combinedGetInfoWithControlSetup" => True
  "name" => <name-1>
  "sessionCorrelationUUID" => "<uuid-1>"
  "groupUUID" => "<uuid-2>"
  "deviceID" => <mac-1>
  "timingProtocol" => "PTP"
  "diagnosticsAndUsage" => True
  "timingPeerInfo" => {
    "ID" => "<uuid-1>"
    "DeviceType" => 0
    "SupportsClockPortMatchingOverride" => True
  }
  "sourceVersion" => "980.77.1"
  "model" => "iPhone17,1"
  "groupContainsGroupLeader" => False
  "statsCollectionEnabled" => False
  "supportsGroupCohesion" => True
  "macAddress" => <mac-2>
  "senderSupportsRelay" => True
  "sessionUUID" => "<uuid-3>"
  "osBuildVersion" => "24A446"
  "updateSessionRequest" => False
  "osName" => "iPhone OS"
  "asyncPTPClockConfig" => True
}
```

#### 7. GET /info

```
X-Apple-ProtocolVersion: 1
CSeq: 6
User-Agent: AirPlay/980.77.1
```

```
{
  "playbackCapabilities" => {
    "supportsInterstitials" => False
    "supportsFPSSecureStop" => False
    "supportsUIForAudioOnlyContent" => False
  }
  "canRecordScreenStream" => False
  "keepAliveSendStatsAsBody" => False
  "protocolVersion" => "1.1"
  "screenDemoMode" => False
  "psi" => <uuid-4>
  "featuresEx" => "AEocQEDDBQA"
  "features" => 1622055604472320
  "statusFlags" => 4
  "deviceID" => <mac-3>
  "pi" => <uuid-5>
  "name" => "zem test"
  "model" => "ShairportSync"
  "pk" => <32-byte public key>
  "senderAddress" => "<ip-1>:59519"
  "initialVolume" => -24.0
  "vv" => 0
  "volumeControlType" => 3
  "supportedFormats" => {
    "audioStream" => 262144
    "bufferStream" => 1649282383872
  }
  "txtAirPlay" => <410 bytes: the receiver's TXT record>
}
```

#### 8. RECORD <session URI>

```
CSeq: 7
User-Agent: AirPlay/980.77.1
```

#### 9. SETPEERS <session URI>

```
Content-Length: 131
Content-Type: /peer-list-changed
CSeq: 8
User-Agent: AirPlay/980.77.1
```

```
[
  0 => "<ip-1>"
  1 => "<ip-2>"
  2 => "<ip-3>"
]
```

#### 10. POST /command

```
Content-Length: 128
Content-Type: application/x-apple-binary-plist
CSeq: 9
User-Agent: AirPlay/980.77.1
```

Body: a `updateMRNowPlayingInfo` command, shown in full below.

#### 11. POST /command

```
Content-Length: 128
Content-Type: application/x-apple-binary-plist
CSeq: 10
User-Agent: AirPlay/980.77.1
```

Body: a `updateMRSupportedCommands` command, shown in full below.

#### 12. POST /command

```
Content-Length: 7211
Content-Type: application/x-apple-binary-plist
CSeq: 11
User-Agent: AirPlay/980.77.1
```

Body: a `updateMRSupportedCommands` command, shown in full below.

#### 13. SETUP <session URI>

```
X-Apple-ProtocolVersion: 1
Content-Length: 309
Content-Type: application/x-apple-binary-plist
CSeq: 12
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "clientID" => "com.apple.Music"
      "type" => 103
      "audioMode" => "default"
      "ct" => 4
      "streamConnectionID" => <stream-id-1>
      "spf" => 1024
      "shk" => <32-byte audio key>
      "supportsDynamicStreamID" => True
      "audioFormat" => 8388608
      "audioFormatIndex" => 23
      "isMedia" => True
    }
  ]
}
```

#### 14. POST /audioMode

```
Content-Length: 64
Content-Type: application/x-apple-binary-plist
CSeq: 13
User-Agent: AirPlay/980.77.1
```

```
{
  "audioMode" => "default"
}
```

#### 15. FLUSHBUFFERED <session URI>

```
Content-Length: 87
Content-Type: application/x-apple-binary-plist
CSeq: 14
User-Agent: AirPlay/980.77.1
```

```
{
  "flushUntilSeq" => 15421490
  "flushUntilTS" => 3001554777
}
```

#### 16. POST /audioMode

```
Content-Length: 64
Content-Type: application/x-apple-binary-plist
CSeq: 15
User-Agent: AirPlay/980.77.1
```

```
{
  "audioMode" => "default"
}
```

#### 17. POST /audioMode

```
Content-Length: 64
Content-Type: application/x-apple-binary-plist
CSeq: 16
User-Agent: AirPlay/980.77.1
```

```
{
  "audioMode" => "default"
}
```

#### 18. SETRATEANCHORTIME <session URI>

```
Content-Length: 190
Content-Type: application/x-apple-binary-plist
CSeq: 17
User-Agent: AirPlay/980.77.1
```

```
{
  "rate" => 1
  "networkTimeTimelineID" => <sender-ptp-clock-1>
  "networkTimeSecs" => 1522601
  "networkTimeFrac" => -7738256952458739712
  "networkTimeFlags" => 0
  "rtpTime" => 4054140669
}
```

#### 19. SET_PARAMETER <session URI>

```
Content-Length: 20
Content-Type: text/parameters
CSeq: 18
User-Agent: AirPlay/980.77.1
```

#### 20. POST /command (inside updateMRSupportedCommands)

```
Content-Length: 212696
Content-Type: application/x-apple-binary-plist
CSeq: 19
User-Agent: AirPlay/980.77.1
```

```
{
  "kMRMediaRemoteCommandInfoIsActiveKey" => True
  "kMRMediaRemoteCommandInfoTransitionStyle" => 1
}
```

#### 21. POST /command

```
Content-Length: 326
Content-Type: application/x-apple-binary-plist
CSeq: 21
User-Agent: AirPlay/980.77.1
```

Body: a `updateMRNowPlayingInfo` command, shown in full below.

#### 22. POST /feedback

```
CSeq: 22
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 23. POST /feedback

```
CSeq: 23
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 24. POST /feedback

```
CSeq: 24
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 25. POST /feedback

```
CSeq: 25
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 26. POST /feedback

```
CSeq: 26
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 27. POST /feedback

```
CSeq: 27
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 28. POST /feedback

```
CSeq: 28
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 29. POST /feedback

```
CSeq: 29
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 30. POST /feedback

```
CSeq: 30
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 31. POST /feedback

```
CSeq: 31
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 32. POST /feedback

```
CSeq: 32
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 33. POST /feedback

```
CSeq: 33
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 34. POST /feedback

```
CSeq: 34
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 35. POST /feedback

```
CSeq: 35
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 36. POST /feedback

```
CSeq: 36
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 37. POST /command

```
Content-Length: 206
Content-Type: application/x-apple-binary-plist
CSeq: 37
User-Agent: AirPlay/980.77.1
```

Body: a `updateMRNowPlayingInfo` command, shown in full below.

#### 38. POST /feedback

```
CSeq: 38
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 39. POST /feedback

```
CSeq: 39
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 40. POST /feedback

```
CSeq: 40
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 41. POST /feedback

```
CSeq: 41
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 42. POST /feedback

```
CSeq: 42
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 43. POST /feedback

```
CSeq: 43
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 44. POST /feedback

```
CSeq: 44
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 45. POST /feedback

```
CSeq: 45
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 46. POST /feedback

```
CSeq: 46
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 47. POST /feedback

```
CSeq: 47
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 48. POST /feedback

```
CSeq: 48
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 49. POST /feedback

```
CSeq: 49
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 50. SETRATEANCHORTIME <session URI>

```
Content-Length: 53
Content-Type: application/x-apple-binary-plist
CSeq: 50
User-Agent: AirPlay/980.77.1
```

```
{
  "rate" => 0
}
```

#### 51. FLUSHBUFFERED <session URI>

```
Content-Length: 87
Content-Type: application/x-apple-binary-plist
CSeq: 51
User-Agent: AirPlay/980.77.1
```

```
{
  "flushUntilSeq" => 15429931
  "flushUntilTS" => 4062797616
}
```

#### 52. POST /audioMode

```
Content-Length: 64
Content-Type: application/x-apple-binary-plist
CSeq: 52
User-Agent: AirPlay/980.77.1
```

```
{
  "audioMode" => "default"
}
```

#### 53. SETRATEANCHORTIME <session URI>

```
Content-Length: 190
Content-Type: application/x-apple-binary-plist
CSeq: 53
User-Agent: AirPlay/980.77.1
```

```
{
  "rate" => 1
  "networkTimeTimelineID" => <sender-ptp-clock-1>
  "networkTimeSecs" => 1522656
  "networkTimeFrac" => -3484421630539595776
  "networkTimeFlags" => 0
  "rtpTime" => 2009646167
}
```

#### 54. POST /command (inside updateMRSupportedCommands)

The same command info again.

#### 55. POST /command

```
Content-Length: 326
Content-Type: application/x-apple-binary-plist
CSeq: 56
User-Agent: AirPlay/980.77.1
```

Body: a `updateMRNowPlayingInfo` command, shown in full below.

#### 56. POST /command

```
Content-Length: 429
Content-Type: application/x-apple-binary-plist
CSeq: 57
User-Agent: AirPlay/980.77.1
```

Body: a `updateMRNowPlayingInfo` command, shown in full below.

#### 57. POST /feedback

```
CSeq: 58
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 58. POST /command

```
Content-Length: 206
Content-Type: application/x-apple-binary-plist
CSeq: 59
User-Agent: AirPlay/980.77.1
```

Body: a `updateMRNowPlayingInfo` command, shown in full below.

#### 59. POST /feedback

```
CSeq: 60
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 60. POST /feedback

```
CSeq: 61
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 61. SETRATEANCHORTIME <session URI>

```
Content-Length: 53
Content-Type: application/x-apple-binary-plist
CSeq: 62
User-Agent: AirPlay/980.77.1
```

```
{
  "rate" => 0
}
```

#### 62. POST /feedback

```
CSeq: 63
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 63. POST /audioMode

```
Content-Length: 64
Content-Type: application/x-apple-binary-plist
CSeq: 64
User-Agent: AirPlay/980.77.1
```

```
{
  "audioMode" => "default"
}
```

#### 64. SETRATEANCHORTIME <session URI>

```
Content-Length: 190
Content-Type: application/x-apple-binary-plist
CSeq: 65
User-Agent: AirPlay/980.77.1
```

```
{
  "rate" => 1
  "networkTimeTimelineID" => <sender-ptp-clock-1>
  "networkTimeSecs" => 1522664
  "networkTimeFrac" => 5990252709490458624
  "networkTimeFlags" => 0
  "rtpTime" => 2009857313
}
```

#### 65. POST /command (inside updateMRSupportedCommands)

The same command info again.

#### 66. POST /command

```
Content-Length: 383
Content-Type: application/x-apple-binary-plist
CSeq: 68
User-Agent: AirPlay/980.77.1
```

Body: a `updateMRNowPlayingInfo` command, shown in full below.

#### 67. POST /feedback

```
CSeq: 69
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 68. SETRATEANCHORTIME <session URI>

```
Content-Length: 53
Content-Type: application/x-apple-binary-plist
CSeq: 70
User-Agent: AirPlay/980.77.1
```

```
{
  "rate" => 0
}
```

#### 69. FLUSHBUFFERED <session URI>

```
Content-Length: 87
Content-Type: application/x-apple-binary-plist
CSeq: 71
User-Agent: AirPlay/980.77.1
```

```
{
  "flushUntilSeq" => 15436094
  "flushUntilTS" => 2015970416
}
```

#### 70. POST /audioMode

```
Content-Length: 64
Content-Type: application/x-apple-binary-plist
CSeq: 72
User-Agent: AirPlay/980.77.1
```

```
{
  "audioMode" => "default"
}
```

#### 71. SETRATEANCHORTIME <session URI>

```
Content-Length: 190
Content-Type: application/x-apple-binary-plist
CSeq: 73
User-Agent: AirPlay/980.77.1
```

```
{
  "rate" => 1
  "networkTimeTimelineID" => <sender-ptp-clock-1>
  "networkTimeSecs" => 1522667
  "networkTimeFrac" => 5192998262709157888
  "networkTimeFlags" => 0
  "rtpTime" => 1258379286
}
```

#### 72. POST /command (inside updateMRSupportedCommands)

The same command info again.

#### 73. POST /feedback

```
CSeq: 76
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 74. POST /command

```
Content-Length: 326
Content-Type: application/x-apple-binary-plist
CSeq: 77
User-Agent: AirPlay/980.77.1
```

Body: a `updateMRNowPlayingInfo` command, shown in full below.

#### 75. POST /feedback

```
CSeq: 78
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 76. POST /feedback

```
CSeq: 79
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 77. SETRATEANCHORTIME <session URI>

```
Content-Length: 53
Content-Type: application/x-apple-binary-plist
CSeq: 80
User-Agent: AirPlay/980.77.1
```

```
{
  "rate" => 0
}
```

#### 78. POST /feedback

```
CSeq: 81
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 79. POST /feedback

```
CSeq: 82
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 80. POST /command

```
Content-Length: 128
Content-Type: application/x-apple-binary-plist
CSeq: 83
User-Agent: AirPlay/980.77.1
```

Body: a `updateMRNowPlayingInfo` command, shown in full below.

#### 81. POST /command

```
Content-Length: 128
Content-Type: application/x-apple-binary-plist
CSeq: 84
User-Agent: AirPlay/980.77.1
```

Body: a `updateMRSupportedCommands` command, shown in full below.

#### 82. POST /command

```
Content-Length: 128
Content-Type: application/x-apple-binary-plist
CSeq: 85
User-Agent: AirPlay/980.77.1
```

Body: a `updateMRNowPlayingInfo` command, shown in full below.

#### 83. POST /command

```
Content-Length: 128
Content-Type: application/x-apple-binary-plist
CSeq: 86
User-Agent: AirPlay/980.77.1
```

Body: a `updateMRSupportedCommands` command, shown in full below.

#### 84. POST /command

```
Content-Length: 5056
Content-Type: application/x-apple-binary-plist
CSeq: 87
User-Agent: AirPlay/980.77.1
```

Body: a `updateMRSupportedCommands` command, shown in full below.

#### 85. TEARDOWN <session URI>

```
Content-Length: 84
Content-Type: application/x-apple-binary-plist
CSeq: 88
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "streamID" => 0
      "type" => 103
    }
  ]
}
```

#### 86. SETUP <session URI>

```
X-Apple-ProtocolVersion: 1
Content-Length: 312
Content-Type: application/x-apple-binary-plist
CSeq: 89
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "clientID" => "com.apple.podcasts"
      "type" => 103
      "audioMode" => "default"
      "ct" => 4
      "streamConnectionID" => <stream-id-2>
      "spf" => 1024
      "shk" => <32-byte audio key>
      "supportsDynamicStreamID" => True
      "audioFormat" => 8388608
      "audioFormatIndex" => 23
      "isMedia" => True
    }
  ]
}
```

#### 87. POST /audioMode

```
Content-Length: 68
Content-Type: application/x-apple-binary-plist
CSeq: 90
User-Agent: AirPlay/980.77.1
```

```
{
  "audioMode" => "spokenAudio"
}
```

#### 88. SETRATEANCHORTIME <session URI>

```
Content-Length: 190
Content-Type: application/x-apple-binary-plist
CSeq: 91
User-Agent: AirPlay/980.77.1
```

```
{
  "rate" => 1
  "networkTimeTimelineID" => <sender-ptp-clock-1>
  "networkTimeSecs" => 1522675
  "networkTimeFrac" => -3683485053947478016
  "networkTimeFlags" => 0
  "rtpTime" => 3311933564
}
```

#### 89. POST

```
Content-Length: 141213
Content-Type: application/x-apple-binary-plist
CSeq: 92
User-Agent: AirPlay/980.77.1
```

```
{
  "MPCRemoteCommandEventOptionVideoActiveKey" => True
}
```

#### 90. POST /command

```
Content-Length: 342
Content-Type: application/x-apple-binary-plist
CSeq: 94
User-Agent: AirPlay/980.77.1
```

Body: a `updateMRNowPlayingInfo` command, shown in full below.

#### 91. POST /feedback

```
CSeq: 95
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 92. POST /feedback

```
CSeq: 96
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 93. POST /feedback

```
CSeq: 97
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 94. POST /feedback

```
CSeq: 98
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 95. POST /feedback

```
CSeq: 99
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 96. SETRATEANCHORTIME <session URI>

```
Content-Length: 53
Content-Type: application/x-apple-binary-plist
CSeq: 100
User-Agent: AirPlay/980.77.1
```

```
{
  "rate" => 0
}
```

#### 97. POST /feedback

```
CSeq: 101
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 98. POST /feedback

```
CSeq: 102
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 99. POST /feedback

```
CSeq: 103
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 100. POST /feedback

```
CSeq: 104
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 101. POST /feedback

```
CSeq: 105
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 102. POST /feedback

```
CSeq: 106
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 103. POST /feedback

```
CSeq: 107
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 104. POST /feedback

```
CSeq: 108
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 105. POST /feedback

```
CSeq: 109
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 106. POST /feedback

```
CSeq: 110
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 107. POST /feedback

```
CSeq: 111
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 108. POST /feedback

```
CSeq: 112
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 109. POST /feedback

```
CSeq: 113
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 110. POST /feedback

```
CSeq: 114
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 111. POST /feedback

```
CSeq: 115
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 112. POST /feedback

```
CSeq: 116
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 113. POST /feedback

```
CSeq: 117
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 114. POST /feedback

```
CSeq: 118
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 115. POST /feedback

```
CSeq: 119
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 116. POST /feedback

```
CSeq: 120
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 117. POST /feedback

```
CSeq: 121
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 118. POST /feedback

```
CSeq: 122
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 119. POST /feedback

```
CSeq: 123
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 120. POST /feedback

```
CSeq: 124
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 121. POST /feedback

```
CSeq: 125
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 122. POST /feedback

```
CSeq: 126
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 123. POST /feedback

```
CSeq: 127
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 124. POST /feedback

```
CSeq: 128
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 125. POST /feedback

```
CSeq: 129
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 126. POST /feedback

```
CSeq: 130
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 127. POST /feedback

```
CSeq: 131
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 128. POST /feedback

```
CSeq: 132
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 129. POST /feedback

```
CSeq: 133
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 130. POST /feedback

```
CSeq: 134
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 131. POST /feedback

```
CSeq: 135
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 132. POST /feedback

```
CSeq: 136
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 133. POST /feedback

```
CSeq: 137
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 134. POST /feedback

```
CSeq: 138
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 135. POST /feedback

```
CSeq: 139
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 136. POST /feedback

```
CSeq: 140
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 137. POST /feedback

```
CSeq: 141
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 138. POST /feedback

```
CSeq: 142
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 139. POST /feedback

```
CSeq: 143
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 140. POST /feedback

```
CSeq: 144
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 141. POST /feedback

```
CSeq: 145
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 142. POST /feedback

```
CSeq: 146
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 143. POST /feedback

```
CSeq: 147
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 144. POST /feedback

```
CSeq: 148
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 145. POST /feedback

```
CSeq: 149
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 146. POST /feedback

```
CSeq: 150
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 147. POST /feedback

```
CSeq: 151
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 148. POST /feedback

```
CSeq: 152
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 149. POST /feedback

```
CSeq: 153
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 150. POST /feedback

```
CSeq: 154
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 151. POST /feedback

```
CSeq: 155
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 152. POST /feedback

```
CSeq: 156
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 153. POST /feedback

```
CSeq: 157
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 154. POST /feedback

```
CSeq: 158
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 155. POST /feedback

```
CSeq: 159
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 156. POST /feedback

```
CSeq: 160
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 157. POST /feedback

```
CSeq: 161
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 158. POST /feedback

```
CSeq: 162
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 159. POST /feedback

```
CSeq: 163
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 160. POST /feedback

```
CSeq: 164
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 161. POST /feedback

```
CSeq: 165
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "type" => 103
      "sr" => 0.0
    }
  ]
}
```

#### 162. FLUSHBUFFERED <session URI>

```
Content-Length: 87
Content-Type: application/x-apple-binary-plist
CSeq: 166
User-Agent: AirPlay/980.77.1
```

```
{
  "flushUntilSeq" => 15481466
  "flushUntilTS" => 3318406324
}
```

#### 163. TEARDOWN <session URI>

```
Content-Length: 84
Content-Type: application/x-apple-binary-plist
CSeq: 167
User-Agent: AirPlay/980.77.1
```

```
{
  "streams" => [
    0 => {
      "streamID" => 0
      "type" => 103
    }
  ]
}
```

#### 164. POST

```
CSeq: 168
User-Agent: AirPlay/980.77.1
```

#### 165. POST

```
CSeq: 169
User-Agent: AirPlay/980.77.1
```

#### 166. POST

```
CSeq: 170
User-Agent: AirPlay/980.77.1
```

#### 167. POST

```
CSeq: 171
User-Agent: AirPlay/980.77.1
```

#### 168. POST

```
CSeq: 172
User-Agent: AirPlay/980.77.1
```

#### 169. POST

```
CSeq: 173
User-Agent: AirPlay/980.77.1
```

#### 170. POST

```
CSeq: 174
User-Agent: AirPlay/980.77.1
```

#### 171. POST

```
CSeq: 175
User-Agent: AirPlay/980.77.1
```

#### 172. POST

```
CSeq: 176
User-Agent: AirPlay/980.77.1
```

#### 173. POST

```
CSeq: 177
User-Agent: AirPlay/980.77.1
```

#### 174. POST

```
CSeq: 178
User-Agent: AirPlay/980.77.1
```

#### 175. POST

```
CSeq: 179
User-Agent: AirPlay/980.77.1
```

#### 176. POST

```
CSeq: 180
User-Agent: AirPlay/980.77.1
```

#### 177. POST

```
CSeq: 181
User-Agent: AirPlay/980.77.1
```

#### 178. POST

```
CSeq: 182
User-Agent: AirPlay/980.77.1
```

#### 179. POST

```
CSeq: 183
User-Agent: AirPlay/980.77.1
```

#### 180. POST

```
CSeq: 184
User-Agent: AirPlay/980.77.1
```

#### 181. POST

```
CSeq: 185
User-Agent: AirPlay/980.77.1
```

#### 182. POST

```
CSeq: 186
User-Agent: AirPlay/980.77.1
```

#### 183. POST

```
CSeq: 187
User-Agent: AirPlay/980.77.1
```

#### 184. POST

```
CSeq: 188
User-Agent: AirPlay/980.77.1
```

#### 185. POST

```
CSeq: 189
User-Agent: AirPlay/980.77.1
```

#### 186. POST

```
CSeq: 190
User-Agent: AirPlay/980.77.1
```

#### 187. POST

```
CSeq: 191
User-Agent: AirPlay/980.77.1
```

#### 188. POST

```
CSeq: 192
User-Agent: AirPlay/980.77.1
```

#### 189. POST

```
CSeq: 193
User-Agent: AirPlay/980.77.1
```

#### 190. POST

```
CSeq: 194
User-Agent: AirPlay/980.77.1
```

#### 191. POST

```
CSeq: 195
User-Agent: AirPlay/980.77.1
```

#### 192. POST

```
CSeq: 196
User-Agent: AirPlay/980.77.1
```

#### 193. POST

```
CSeq: 197
User-Agent: AirPlay/980.77.1
```

#### 194. POST

```
CSeq: 198
User-Agent: AirPlay/980.77.1
```

#### 195. POST

```
CSeq: 199
User-Agent: AirPlay/980.77.1
```

#### 196. POST

```
CSeq: 200
User-Agent: AirPlay/980.77.1
```

#### 197. POST

```
CSeq: 201
User-Agent: AirPlay/980.77.1
```

#### 198. POST

```
CSeq: 202
User-Agent: AirPlay/980.77.1
```

#### 199. POST

```
CSeq: 203
User-Agent: AirPlay/980.77.1
```

### POST /command bodies, in order

```
{
  "type" => "updateMRNowPlayingInfo"
  "params" => {
    "type" => "npi-text"
    "params" => {}
    "mergePolicy" => "replace"
  }
}
```

```
{
  "type" => "updateMRSupportedCommands"
  "params" => {
    "mrSupportedCommandsFromSender" => []
  }
}
```

```
{
  "type" => "updateMRSupportedCommands"
  "params" => {
    "mrSupportedCommandsFromSender" => [
      0 => <binary plist> {
        "kCommandInfoCommandKey" => 25021
        "kCommandInfoEnabledKey" => True
      }
      1 => <binary plist> {
        "kCommandInfoCommandKey" => 25020
        "kCommandInfoEnabledKey" => True
      }
      2 => <binary plist> {
        "kCommandInfoOptionsKey" => {
          "kMRMediaRemoteCommandInfoIsActiveKey" => True
          "kMRMediaRemoteCommandInfoTransitionStyle" => 1
        }
        "kCommandInfoCommandKey" => 149
        "kCommandInfoEnabledKey" => True
      }
      3 => <binary plist> {
        "kCommandInfoCommandKey" => 145
        "kCommandInfoEnabledKey" => True
      }
      4 => <binary plist> {
        "kCommandInfoCommandKey" => 143
        "kCommandInfoEnabledKey" => True
      }
      5 => <binary plist> {
        "kCommandInfoOptionsKey" => {
          "kMRMediaRemoteCommandInfoCurrentQueueEndAction" => 3
          "kMRMediaRemoteCommandInfoSupportedQueueEndActions" => [
            0 => 2
            1 => 3
          ]
        }
        "kCommandInfoCommandKey" => 135
        "kCommandInfoEnabledKey" => True
      }
      6 => <binary plist> {
        "kCommandInfoCommandKey" => 134
        "kCommandInfoEnabledKey" => True
      }
      7 => <binary plist> {
        "kCommandInfoOptionsKey" => {
          "kMRMediaRemoteCommandInfoCurrentPlaybackSessionTypes" => [
            0 => "com.apple.MediaPlaybackCore.playbackSession-v4.5.opack:subscription:cloudLibrary:6fe7557"
            1 => "com.apple.MediaPlaybackCore.playbackSession-v4.5.opack:subscription:storeFront:<storefront-1>"
          ]
          "kMRMediaRemoteCommandInfoSupportedPlaybackSessionTypes" => [
            0 => "com.apple.MediaPlaybackCore.playbackSession-v4.5.opack:subscription:storeFront:<storefront-1>"
          ]
          "kMRMediaRemoteCommandInfoPlaybackSessionRequirements" => {
            "filter" => <148 bytes>
            "features" => 2
          }
          "kMRMediaRemoteCommandInfoPlaybackSessionIdentifier" => <session-id-1>
        }
        "kCommandInfoCommandKey" => 133
        "kCommandInfoEnabledKey" => True
      }
      8 => <binary plist> {
        "kCommandInfoCommandKey" => 132
        "kCommandInfoEnabledKey" => True
      }
      9 => <binary plist> {
        "kCommandInfoCommandKey" => 131
        "kCommandInfoEnabledKey" => True
      }
      10 => <binary plist> {
        "kCommandInfoCommandKey" => 130
        "kCommandInfoEnabledKey" => True
      }
      11 => <binary plist> {
        "kCommandInfoCommandKey" => 129
        "kCommandInfoEnabledKey" => True
      }
      12 => <binary plist> {
        "kCommandInfoCommandKey" => 128
        "kCommandInfoEnabledKey" => True
      }
      13 => <binary plist> {
        "kCommandInfoOptionsKey" => {
          "kMRMediaRemoteCommandInfoIsActiveKey" => False
        }
        "kCommandInfoCommandKey" => 127
        "kCommandInfoEnabledKey" => False
      }
      14 => <binary plist> {
        "kCommandInfoOptionsKey" => {
          "kMRMediaRemoteCommandInfoSupportedPlaybackQueueTypes" => [
            0 => 7
            1 => 3
            2 => 6
            3 => 2
            4 => 5
            5 => 1
          ]
          "kMRMediaRemoteCommandInfoSupportedCustomPlaybackQueueIdentifiers" => [
            0 => "com.apple.music.playbackqueue.tracklist"
            1 => "com.apple.music.playbackqueue.radio"
            2 => "com.apple.mediaplayer.playbackcontext"
          ]
          "kMRMediaRemoteCommandInfoSupportedInsertionPositions" => [
            0 => 0
            1 => 3
          ]
        }
        "kCommandInfoCommandKey" => 125
        "kCommandInfoEnabledKey" => True
      }
      15 => <binary plist> {
        "kCommandInfoOptionsKey" => {
          "kMRMediaRemoteCommandInfoSupportedPlaybackQueueTypes" => [
            0 => 7
            1 => 3
            2 => 6
            3 => 2
            4 => 5
            5 => 1
            6 => 8
          ]
          "kMRMediaRemoteCommandInfoSupportedCustomPlaybackQueueIdentifiers" => [
            0 => "com.apple.music.playbackqueue.sharedlistening"
            1 => "com.apple.music.playbackqueue.radio"
            2 => "com.apple.music.playbackqueue.tracklist"
            3 => "com.apple.MediaPlaybackCore.playbackContextArchive-v1.opack"
            4 => "com.apple.mediaplayer.playbackcontext"
          ]
        }
        "kCommandInfoCommandKey" => 122
        "kCommandInfoEnabledKey" => True
      }
      16 => <binary plist> {
        "kCommandInfoCommandKey" => 121
        "kCommandInfoEnabledKey" => True
      }
      17 => <binary plist> {
        "kCommandInfoOptionsKey" => {
          "kMRMediaRemoteCommandInfoShuffleMode" => 1
        }
        "kCommandInfoCommandKey" => 26
        "kCommandInfoEnabledKey" => True
      }
      18 => <binary plist> {
        "kCommandInfoOptionsKey" => {
          "kMRMediaRemoteCommandInfoRepeatMode" => 1
        }
        "kCommandInfoCommandKey" => 25
        "kCommandInfoEnabledKey" => True
      }
      19 => <binary plist> {
        "kCommandInfoOptionsKey" => {
          "kMRMediaRemoteCommandInfoCanBeControlledByScrubbingKey" => True
          "kMRMediaRemoteCommandInfoSupportsReferencePosition" => False
        }
        "kCommandInfoCommandKey" => 24
        "kCommandInfoEnabledKey" => True
      }
      20 => <binary plist> {
        "kCommandInfoOptionsKey" => {
          "kMRMediaRemoteCommandInfoIsActiveKey" => False
          "kMRMediaRemoteCommandInfoPresentationStyleKey" => 2
          "kMRMediaRemoteCommandInfoLocalizedShortTitleKey" => "Never Play"
          "kMRMediaRemoteCommandInfoLocalizedTitleKey" => "Play Less Like This"
        }
        "kCommandInfoCommandKey" => 22
        "kCommandInfoEnabledKey" => True
      }
      21 => <binary plist> {
        "kCommandInfoOptionsKey" => {
          "kMRMediaRemoteCommandInfoIsActiveKey" => False
          "kMRMediaRemoteCommandInfoPresentationStyleKey" => 2
          "kMRMediaRemoteCommandInfoLocalizedShortTitleKey" => "Like"
          "kMRMediaRemoteCommandInfoLocalizedTitleKey" => "Play More Like This"
        }
        "kCommandInfoCommandKey" => 21
        "kCommandInfoEnabledKey" => True
      }
      22 => <binary plist> {
        "kCommandInfoCommandKey" => 19
        "kCommandInfoEnabledKey" => False
      }
      23 => <binary plist> {
        "kCommandInfoOptionsKey" => {
          "kMRMediaRemoteCommandInfoPreferredIntervalsKey" => [
            0 => 30
          ]
        }
        "kCommandInfoCommandKey" => 18
        "kCommandInfoEnabledKey" => False
      }
      24 => <binary plist> {
        "kCommandInfoOptionsKey" => {
          "kMRMediaRemoteCommandInfoPreferredIntervalsKey" => [
            0 => 30
          ]
        }
        "kCommandInfoCommandKey" => 17
        "kCommandInfoEnabledKey" => False
      }
      25 => <binary plist> {
        "kCommandInfoCommandKey" => 10
        "kCommandInfoEnabledKey" => True
      }
      26 => <binary plist> {
        "kCommandInfoCommandKey" => 11
        "kCommandInfoEnabledKey" => True
      }
      27 => <binary plist> {
        "kCommandInfoCommandKey" => 8
        "kCommandInfoEnabledKey" => True
      }
      28 => <binary plist> {
        "kCommandInfoCommandKey" => 9
        "kCommandInfoEnabledKey" => True
      }
      29 => <binary plist> {
        "kCommandInfoOptionsKey" => {
          "kMRMediaRemoteCommandInfoRepeatMode" => 1
        }
        "kCommandInfoCommandKey" => 7
        "kCommandInfoEnabledKey" => True
      }
      30 => <binary plist> {
        "kCommandInfoOptionsKey" => {
          "kMRMediaRemoteCommandInfoShuffleMode" => 1
        }
        "kCommandInfoCommandKey" => 6
        "kCommandInfoEnabledKey" => True
      }
      31 => <binary plist> {
        "kCommandInfoCommandKey" => 5
        "kCommandInfoEnabledKey" => True
      }
      32 => <binary plist> {
        "kCommandInfoCommandKey" => 4
        "kCommandInfoEnabledKey" => True
      }
      33 => <binary plist> {
        "kCommandInfoCommandKey" => 3
        "kCommandInfoEnabledKey" => True
      }
      34 => <binary plist> {
        "kCommandInfoCommandKey" => 2
        "kCommandInfoEnabledKey" => True
      }
      35 => <binary plist> {
        "kCommandInfoCommandKey" => 1
        "kCommandInfoEnabledKey" => True
      }
      36 => <binary plist> {
        "kCommandInfoCommandKey" => 0
        "kCommandInfoEnabledKey" => True
      }
    ]
  }
}
```

(shairport-sync: `pbeg`)

(shairport-sync: `pres`)

```
{
  "type" => "updateMRNowPlayingInfo"
  "params" => {
    "type" => "npi-text"
    "params" => {
      "kMRMediaRemoteNowPlayingInfoArtworkIdentifier" => <artwork-url-1>
      "kMRMediaRemoteNowPlayingInfoTrackNumber" => 2
      "kMRMediaRemoteNowPlayingInfoDuration" => 261.49755102040814
      "kMRMediaRemoteNowPlayingInfoContentType" => "music"
      "kMRMediaRemoteNowPlayingInfoTitle" => <title-1>
      "kMRMediaRemoteNowPlayingInfoPlaybackRate" => 1.0
      "kMRMediaRemoteNowPlayingInfoStrictMediaType" => "audio"
      "kMRMediaRemoteNowPlayingInfoAlbum" => <album-1>
      "kMRMediaRemoteNowPlayingInfoTotalQueueCount" => 50
      "kMRMediaRemoteNowPlayingInfoArtistiTunesStoreAdamIdentifier" => <store-id-1>
      "kMRMediaRemoteNowPlayingInfoUserInfo" => {
        "hsStLy" => True
        "subReq" => True
        "libEligible" => True
        "hsTSL" => True
        "sfid" => <storefront-2>
        "rdwn" => True
        "endT" => 261.49755102040814
        "lcd" => 0
      }
      "kMRMediaRemoteNowPlayingInfoArtworkData" => <600x600 JPEG, 210328 bytes>
      "kMRMediaRemoteNowPlayingInfoMediaType" => "MRMediaRemoteMediaTypeMusic"
      "kMRMediaRemoteNowPlayingInfoiTunesStoreSubscriptionAdamIdentifier" => <store-id-2>
      "kMRMediaRemoteNowPlayingInfoCollectionInfo" => {}
      "kMRMediaRemoteNowPlayingInfoTimestamp" => 2026-11-01T23:18:33.400642Z
      "kMRMediaRemoteNowPlayingInfoGenre" => <genre-1>
      "kMRMediaRemoteNowPlayingInfoComposer" => <composer-1>
      "kMRMediaRemoteNowPlayingInfoQueueIndex" => 0
      "kMRMediaRemoteNowPlayingInfoIsAlwaysLive" => False
      "kMRMediaRemoteNowPlayingInfoArtist" => <artist-1>
      "kMRMediaRemoteNowPlayingInfoiTunesStoreIdentifier" => <store-id-2>
      "kMRMediaRemoteNowPlayingInfoDefaultPlaybackRate" => 1.0
      "kMRMediaRemoteNowPlayingInfoElapsedTime" => 5.945328922
      "kMRMediaRemoteNowPlayingInfoArtworkDataWidth" => 600
      "kMRMediaRemoteNowPlayingInfoContentItemIdentifier" => <item-1>
      "kMRMediaRemoteNowPlayingInfoIsInTransition" => False
      "kMRMediaRemoteNowPlayingInfoArtworkDataHeight" => 600
      "kMRMediaRemoteNowPlayingInfoAlbumiTunesStoreAdamIdentifier" => <store-id-3>
      "kMRMediaRemoteNowPlayingInfoUniqueIdentifier" => <unique-id-1>
    }
    "mergePolicy" => "replace"
  }
}
```

updateMRSupportedCommands again, 37 commands.

```
{
  "type" => "updateMRNowPlayingInfo"
  "params" => {
    "type" => "npi-text"
    "params" => {
      "kMRMediaRemoteNowPlayingInfoTimestamp" => 2026-11-01T23:18:33.400642Z
      "kMRMediaRemoteNowPlayingInfoPlaybackRate" => 1.0
      "kMRMediaRemoteNowPlayingInfoContentItemIdentifier" => <item-1>
    }
    "mergePolicy" => "update"
  }
}
```

```
{
  "type" => "updateMRNowPlayingInfo"
  "params" => {
    "type" => "npi-text"
    "params" => {
      "kMRMediaRemoteNowPlayingInfoContentItemIdentifier" => <item-1>
    }
    "mergePolicy" => "update"
  }
}
```

(shairport-sync: `paus`)

(shairport-sync: `pres`)

```
{
  "type" => "updateMRNowPlayingInfo"
  "params" => {
    "type" => "npi-text"
    "params" => {
      "kMRMediaRemoteNowPlayingInfoArtworkIdentifier" => <artwork-url-2>
      "kMRMediaRemoteNowPlayingInfoTrackNumber" => 13
      "kMRMediaRemoteNowPlayingInfoDuration" => 178.396
      "kMRMediaRemoteNowPlayingInfoContentType" => "music"
      "kMRMediaRemoteNowPlayingInfoTitle" => <title-2>
      "kMRMediaRemoteNowPlayingInfoPlaybackRate" => 0.0
      "kMRMediaRemoteNowPlayingInfoStrictMediaType" => "audio"
      "kMRMediaRemoteNowPlayingInfoAlbum" => <album-2>
      "kMRMediaRemoteNowPlayingInfoTotalQueueCount" => 50
      "kMRMediaRemoteNowPlayingInfoArtistiTunesStoreAdamIdentifier" => <store-id-4>
      "kMRMediaRemoteNowPlayingInfoUserInfo" => {
        "hsStLy" => True
        "subReq" => True
        "libEligible" => True
        "rdwn" => True
        "sfid" => <storefront-2>
        "lcd" => 1
        "endT" => 178.399
      }
      "kMRMediaRemoteNowPlayingInfoArtworkData" => <600x600 JPEG, 77032 bytes>
      "kMRMediaRemoteNowPlayingInfoMediaType" => "MRMediaRemoteMediaTypeMusic"
      "kMRMediaRemoteNowPlayingInfoiTunesStoreSubscriptionAdamIdentifier" => <store-id-5>
      "kMRMediaRemoteNowPlayingInfoCollectionInfo" => {}
      "kMRMediaRemoteNowPlayingInfoTimestamp" => 2026-11-01T23:19:28.433728Z
      "kMRMediaRemoteNowPlayingInfoGenre" => <genre-2>
      "kMRMediaRemoteNowPlayingInfoComposer" => <composer-2>
      "kMRMediaRemoteNowPlayingInfoQueueIndex" => 1
      "kMRMediaRemoteNowPlayingInfoIsAlwaysLive" => False
      "kMRMediaRemoteNowPlayingInfoArtist" => <artist-2>
      "kMRMediaRemoteNowPlayingInfoiTunesStoreIdentifier" => <store-id-5>
      "kMRMediaRemoteNowPlayingInfoDefaultPlaybackRate" => 1.0
      "kMRMediaRemoteNowPlayingInfoElapsedTime" => 0.0
      "kMRMediaRemoteNowPlayingInfoArtworkDataWidth" => 600
      "kMRMediaRemoteNowPlayingInfoContentItemIdentifier" => <item-2>
      "kMRMediaRemoteNowPlayingInfoIsInTransition" => False
      "kMRMediaRemoteNowPlayingInfoArtworkDataHeight" => 600
      "kMRMediaRemoteNowPlayingInfoAlbumiTunesStoreAdamIdentifier" => <store-id-6>
      "kMRMediaRemoteNowPlayingInfoUniqueIdentifier" => <unique-id-2>
    }
    "mergePolicy" => "replace"
  }
}
```

updateMRSupportedCommands again, 37 commands.

```
{
  "type" => "updateMRNowPlayingInfo"
  "params" => {
    "type" => "npi-text"
    "params" => {
      "kMRMediaRemoteNowPlayingInfoTimestamp" => 2026-11-01T23:19:28.630818Z
      "kMRMediaRemoteNowPlayingInfoPlaybackRate" => 1.0
      "kMRMediaRemoteNowPlayingInfoContentItemIdentifier" => <item-2>
    }
    "mergePolicy" => "update"
  }
}
```

```
{
  "type" => "updateMRNowPlayingInfo"
  "params" => {
    "type" => "npi-text"
    "params" => {
      "kMRMediaRemoteNowPlayingInfoUserInfo" => {
        "hsStLy" => True
        "subReq" => True
        "libEligible" => True
        "rdwn" => True
        "sfid" => <storefront-2>
        "lcd" => 1
        "endT" => 178.396
      }
      "kMRMediaRemoteNowPlayingInfoCollectionInfo" => {}
      "kMRMediaRemoteNowPlayingInfoContentItemIdentifier" => <item-2>
    }
    "mergePolicy" => "update"
  }
}
```

```
{
  "type" => "updateMRNowPlayingInfo"
  "params" => {
    "type" => "npi-text"
    "params" => {
      "kMRMediaRemoteNowPlayingInfoContentItemIdentifier" => <item-2>
    }
    "mergePolicy" => "update"
  }
}
```

(shairport-sync: `paus`)

(shairport-sync: `pres`)

```
{
  "type" => "updateMRNowPlayingInfo"
  "params" => {
    "type" => "npi-text"
    "params" => {
      "kMRMediaRemoteNowPlayingInfoArtworkIdentifier" => <artwork-url-2>
      "kMRMediaRemoteNowPlayingInfoTrackNumber" => 13
      "kMRMediaRemoteNowPlayingInfoDuration" => 178.396
      "kMRMediaRemoteNowPlayingInfoContentType" => "music"
      "kMRMediaRemoteNowPlayingInfoTitle" => <title-2>
      "kMRMediaRemoteNowPlayingInfoPlaybackRate" => 0.0
      "kMRMediaRemoteNowPlayingInfoStrictMediaType" => "audio"
      "kMRMediaRemoteNowPlayingInfoAlbum" => <album-2>
      "kMRMediaRemoteNowPlayingInfoTotalQueueCount" => 50
      "kMRMediaRemoteNowPlayingInfoArtistiTunesStoreAdamIdentifier" => <store-id-4>
      "kMRMediaRemoteNowPlayingInfoUserInfo" => {
        "hsStLy" => True
        "subReq" => True
        "libEligible" => True
        "rdwn" => True
        "sfid" => <storefront-2>
        "lcd" => 1
        "endT" => 178.396
      }
      "kMRMediaRemoteNowPlayingInfoArtworkData" => <600x600 JPEG, 77032 bytes>
      "kMRMediaRemoteNowPlayingInfoMediaType" => "MRMediaRemoteMediaTypeMusic"
      "kMRMediaRemoteNowPlayingInfoiTunesStoreSubscriptionAdamIdentifier" => <store-id-5>
      "kMRMediaRemoteNowPlayingInfoCollectionInfo" => {}
      "kMRMediaRemoteNowPlayingInfoTimestamp" => 2026-11-01T23:19:33.628859Z
      "kMRMediaRemoteNowPlayingInfoGenre" => <genre-2>
      "kMRMediaRemoteNowPlayingInfoComposer" => <composer-2>
      "kMRMediaRemoteNowPlayingInfoQueueIndex" => 1
      "kMRMediaRemoteNowPlayingInfoIsAlwaysLive" => False
      "kMRMediaRemoteNowPlayingInfoArtist" => <artist-2>
      "kMRMediaRemoteNowPlayingInfoiTunesStoreIdentifier" => <store-id-5>
      "kMRMediaRemoteNowPlayingInfoDefaultPlaybackRate" => 1.0
      "kMRMediaRemoteNowPlayingInfoElapsedTime" => 4.698173542
      "kMRMediaRemoteNowPlayingInfoArtworkDataWidth" => 600
      "kMRMediaRemoteNowPlayingInfoContentItemIdentifier" => <item-2>
      "kMRMediaRemoteNowPlayingInfoIsInTransition" => False
      "kMRMediaRemoteNowPlayingInfoArtworkDataHeight" => 600
      "kMRMediaRemoteNowPlayingInfoAlbumiTunesStoreAdamIdentifier" => <store-id-6>
      "kMRMediaRemoteNowPlayingInfoUniqueIdentifier" => <unique-id-2>
    }
    "mergePolicy" => "replace"
  }
}
```

updateMRSupportedCommands again, 37 commands.

```
{
  "type" => "updateMRNowPlayingInfo"
  "params" => {
    "type" => "npi-text"
    "params" => {
      "kMRMediaRemoteNowPlayingInfoTimestamp" => 2026-11-01T23:19:36.144319Z
      "kMRMediaRemoteNowPlayingInfoPlaybackRate" => 1.0
      "kMRMediaRemoteNowPlayingInfoContentItemIdentifier" => <item-2>
      "kMRMediaRemoteNowPlayingInfoElapsedTime" => 4.699595332
    }
    "mergePolicy" => "update"
  }
}
```

(shairport-sync: `paus`)

(shairport-sync: `pres`)

```
{
  "type" => "updateMRNowPlayingInfo"
  "params" => {
    "type" => "npi-text"
    "params" => {
      "kMRMediaRemoteNowPlayingInfoArtworkIdentifier" => <artwork-url-2>
      "kMRMediaRemoteNowPlayingInfoTrackNumber" => 13
      "kMRMediaRemoteNowPlayingInfoDuration" => 178.396
      "kMRMediaRemoteNowPlayingInfoContentType" => "music"
      "kMRMediaRemoteNowPlayingInfoTitle" => <title-2>
      "kMRMediaRemoteNowPlayingInfoPlaybackRate" => 0.0
      "kMRMediaRemoteNowPlayingInfoStrictMediaType" => "audio"
      "kMRMediaRemoteNowPlayingInfoAlbum" => <album-2>
      "kMRMediaRemoteNowPlayingInfoTotalQueueCount" => 50
      "kMRMediaRemoteNowPlayingInfoArtistiTunesStoreAdamIdentifier" => <store-id-4>
      "kMRMediaRemoteNowPlayingInfoUserInfo" => {
        "hsStLy" => True
        "subReq" => True
        "libEligible" => True
        "rdwn" => True
        "sfid" => <storefront-2>
        "lcd" => 1
        "endT" => 178.396
      }
      "kMRMediaRemoteNowPlayingInfoArtworkData" => <600x600 JPEG, 77032 bytes>
      "kMRMediaRemoteNowPlayingInfoMediaType" => "MRMediaRemoteMediaTypeMusic"
      "kMRMediaRemoteNowPlayingInfoiTunesStoreSubscriptionAdamIdentifier" => <store-id-5>
      "kMRMediaRemoteNowPlayingInfoCollectionInfo" => {}
      "kMRMediaRemoteNowPlayingInfoTimestamp" => 2026-11-01T23:19:38.592576Z
      "kMRMediaRemoteNowPlayingInfoGenre" => <genre-2>
      "kMRMediaRemoteNowPlayingInfoComposer" => <composer-2>
      "kMRMediaRemoteNowPlayingInfoQueueIndex" => 1
      "kMRMediaRemoteNowPlayingInfoIsAlwaysLive" => False
      "kMRMediaRemoteNowPlayingInfoArtist" => <artist-2>
      "kMRMediaRemoteNowPlayingInfoiTunesStoreIdentifier" => <store-id-5>
      "kMRMediaRemoteNowPlayingInfoDefaultPlaybackRate" => 1.0
      "kMRMediaRemoteNowPlayingInfoElapsedTime" => 88.597
      "kMRMediaRemoteNowPlayingInfoArtworkDataWidth" => 600
      "kMRMediaRemoteNowPlayingInfoContentItemIdentifier" => <item-2>
      "kMRMediaRemoteNowPlayingInfoIsInTransition" => False
      "kMRMediaRemoteNowPlayingInfoArtworkDataHeight" => 600
      "kMRMediaRemoteNowPlayingInfoAlbumiTunesStoreAdamIdentifier" => <store-id-6>
      "kMRMediaRemoteNowPlayingInfoUniqueIdentifier" => <unique-id-2>
    }
    "mergePolicy" => "replace"
  }
}
```

updateMRSupportedCommands again, 37 commands.

```
{
  "type" => "updateMRNowPlayingInfo"
  "params" => {
    "type" => "npi-text"
    "params" => {
      "kMRMediaRemoteNowPlayingInfoTimestamp" => 2026-11-01T23:19:39.102760Z
      "kMRMediaRemoteNowPlayingInfoPlaybackRate" => 1.0
      "kMRMediaRemoteNowPlayingInfoContentItemIdentifier" => <item-2>
    }
    "mergePolicy" => "update"
  }
}
```

(shairport-sync: `paus`)

```
{
  "type" => "updateMRNowPlayingInfo"
  "params" => {
    "type" => "npi-text"
    "params" => {}
    "mergePolicy" => "replace"
  }
}
```

```
{
  "type" => "updateMRSupportedCommands"
  "params" => {
    "mrSupportedCommandsFromSender" => []
  }
}
```

```
{
  "type" => "updateMRNowPlayingInfo"
  "params" => {
    "type" => "npi-text"
    "params" => {}
    "mergePolicy" => "replace"
  }
}
```

```
{
  "type" => "updateMRSupportedCommands"
  "params" => {
    "mrSupportedCommandsFromSender" => []
  }
}
```

updateMRSupportedCommands again, 28 commands.

(shairport-sync: `pend`)

(shairport-sync: `pbeg`)

(shairport-sync: `pres`)

```
{
  "type" => "updateMRNowPlayingInfo"
  "params" => {
    "type" => "npi-text"
    "params" => {
      "kMRMediaRemoteNowPlayingInfoArtworkIdentifier" => <artwork-url-3>
      "kMRMediaRemoteNowPlayingInfoDuration" => 1366.2302040816326
      "kMRMediaRemoteNowPlayingInfoContentType" => "podcast"
      "kMRMediaRemoteNowPlayingInfoTitle" => <title-3>
      "kMRMediaRemoteNowPlayingInfoPlaybackRate" => 0.0
      "kMRMediaRemoteNowPlayingInfoStrictMediaType" => "audio"
      "kMRMediaRemoteNowPlayingInfoAlbum" => <album-3>
      "kMRMediaRemoteNowPlayingInfoTotalQueueCount" => 10
      "kMRMediaRemoteNowPlayingInfoUserInfo" => {
        "podEpPrTy" => "STDQ"
        "podSort" => 1
        "podEpDownMK" => []
        "podEpTrId" => <transcript-1>
        "podEpTrSrc" => "apple"
        "lcd" => 0
        "podArtInfo" => {
          "artwork-info-show-artwork" => <316 bytes: 7b227175617465726e61727954657874...>
          "artwork-info-uber-artwork" => <326 bytes: 7b22746572746961727954657874436f...>
          "artwork-info-episode-artwork" => <340 bytes: 7b227769647468223a3630302c226261...>
        }
        "podShoChapArt" => False
        "podEpStrURL" => "<url-1>"
        "podArtURL" => "<url-2>"
        "podEpShURL" => "<url-3>"
        "endT" => 1366.2302040816326
        "podURL" => "<url-4>"
        "podEpChapGen" => True
        "podSupSub" => True
      }
      "kMRMediaRemoteNowPlayingInfoArtworkData" => <600x600 JPEG, 137168 bytes>
      "kMRMediaRemoteNowPlayingInfoMediaType" => "MRMediaRemoteMediaTypePodcast"
      "kMRMediaRemoteNowPlayingInfoiTunesStoreSubscriptionAdamIdentifier" => <store-id-7>
      "kMRMediaRemoteNowPlayingInfoCollectionInfo" => {}
      "kMRMediaRemoteNowPlayingInfoTimestamp" => 2026-11-01T23:19:47.263952Z
      "kMRMediaRemoteNowPlayingInfoGenre" => <genre-3>
      "kMRMediaRemoteNowPlayingInfoComposer" => <composer-3>
      "kMRMediaRemoteNowPlayingInfoQueueIndex" => 0
      "kMRMediaRemoteNowPlayingInfoIsAlwaysLive" => False
      "kMRMediaRemoteNowPlayingInfoArtist" => <artist-3>
      "kMRMediaRemoteNowPlayingInfoiTunesStoreIdentifier" => <store-id-8>
      "kMRMediaRemoteNowPlayingInfoDefaultPlaybackRate" => 1.0
      "kMRMediaRemoteNowPlayingInfoElapsedTime" => 0.0
      "kMRMediaRemoteNowPlayingInfoArtworkDataWidth" => 600
      "kMRMediaRemoteNowPlayingInfoContentItemIdentifier" => <item-3>
      "kMRMediaRemoteNowPlayingInfoIsInTransition" => False
      "kMRMediaRemoteNowPlayingInfoExternalContentIdentifier" => "<uuid-6>"
      "kMRMediaRemoteNowPlayingInfoAlbumiTunesStoreAdamIdentifier" => <store-id-7>
      "kMRMediaRemoteNowPlayingInfoUniqueIdentifier" => <unique-id-3>
      "kMRMediaRemoteNowPlayingInfoArtworkDataHeight" => 600
    }
    "mergePolicy" => "replace"
  }
}
```

updateMRSupportedCommands again, 28 commands.

```
{
  "type" => "updateMRNowPlayingInfo"
  "params" => {
    "type" => "npi-text"
    "params" => {
      "kMRMediaRemoteNowPlayingInfoTimestamp" => 2026-11-01T23:19:47.619838Z
      "kMRMediaRemoteNowPlayingInfoPlaybackRate" => 1.0
      "kMRMediaRemoteNowPlayingInfoContentItemIdentifier" => <item-3>
    }
    "mergePolicy" => "update"
  }
}
```

(shairport-sync: `paus`)

(shairport-sync: `pend`)

