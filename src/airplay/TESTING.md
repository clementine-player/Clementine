# AirPlay receiver tests

Results of playing to real receivers, beyond the fake receiver in
`tests/fakeairplayreceiver.cpp`.

## Mac (AirTunes 980.77.5)

A Mac14,2 with AirPlay Receiver set to allow anyone on the network, tested
2026-10-30 and 2026-11-01. It advertises both `_airplay._tcp` and
`_raop._tcp`, under the same MAC.

### Discovery and pairing

| Check | Result |
| --- | --- |
| Discovery | Found over `_airplay._tcp`. |
| Status flags | 0x84 (or 0x4); 0x284 with "Require password" on, so 0x200 means a password. |
| Transient pairing | Works with PIN 3939, without /pair-pin-start. |
| /pair-pin-start | Asks to Allow, then shows a full-screen PIN: 3939 on 2026-10-30, a random one on 2026-11-01. |
| Allow prompt | Holds the first SETUP until answered; unanswered, the Mac answers 400 after about 15 s. |
| Asking again | It remembers allowing a sender for a while, but asks again at times for reasons unknown, even a minute later or after a clean TEARDOWN. |

### Playing

| Check | Result |
| --- | --- |
| Tone | Clean. |
| Volume | Right: 20·log10 of the gain, so 50% on the slider is about −12 dB. |
| Pause, seek, skip, gapless | All clean. |
| Unanswered Allow | Stays on the receiver, stopped, with a message. |
| Missed packets, 2026-11-03 | A Mac asked again for about 35 packets at the start of a session. Wrapped in RAOP's retransmit on the control channel, none decrypted (`kAuthenticationErr` in its log): 35 unrecovered, and a glitch. Sent again as they were on the audio channel, as OwnTone does: none unrecovered, and no glitch. |

### Passwords and long-term pairing

| Check | Result |
| --- | --- |
| No password given | Seen from GET /info before pairing; nothing played. |
| Wrong password | Pair-setup M4 has error 0x02 (authentication); reported as a wrong password. |
| Right password | Paired, asked to Allow, then played. |
| Turning the password off | The Mac advertised no password at once, but kept pairing only with the old one until System Settings was closed. |
| Too many failed attempts | Pair-setup refused with error 0x03 (backoff), with a retry delay of a few seconds. |
| Long-term pairing, with a password | M5/M6 accepted, but on a later connection SETUP got 401 after pair-verify: the password is needed every time. |
| Long-term pairing with 3939, no password | Refused with error 0x02: 3939 is only for transient pairing. |
| Long-term pairing with the PIN shown, no password | Worked; a later connection pair-verified with no PIN and wasn't asked to Allow. |

So without a password, pairing once with the PIN the Mac shows gives keys to
connect with afterwards. Whether that stops it asking to Allow again from
time to time needs watching for longer.

### AirPlay 1

| Check | Result |
| --- | --- |
| Discovery | `_raop._tcp` gives the same id as `_airplay._tcp`. |
| ANNOUNCE, with a password | 401 with a Digest challenge (realm "airplay") on 2026-11-01; 403 on 2026-10-30. |
| Without a password | ANNOUNCE, SETUP, RECORD, FLUSH and SET_PARAMETER all accepted, timing requests come, and nothing plays. |
| After pairing over AirPlay 2 | The Allow prompt holds ANNOUNCE, then the same: accepted, and nothing plays. |
| The proof of concept (#7653) | Accepted the same way, and nothing played, though it played on 2026-10-30. |

So the Mac doesn't play AirPlay 1 audio, at least now; AirPlay 1 is for
receivers that only have it.

### Now playing, over AirPlay 2

| Check | Result |
| --- | --- |
| Title, artist, album | Shown, from DMAP in SET_PARAMETER, and updated for the next song. |
| Cover | Shown, from a JPEG in SET_PARAMETER. |
| A new cover for the next song | Accepted, but the first cover stayed for the rest of the session, with a new DMAP persistent id ("mper") or resent once the song was playing. `image/none` didn't clear it either. |
| Progress | Applied when it changed (a seek showed 0:20 of 1:00), but never counted up, and Now Playing showed a Play button, as if paused. The same with progress as OwnTone sends it. |

So with SET_PARAMETER alone, the Mac shows what's playing and applies
seeks, but shows it as paused and keeps the first song's cover.

Since then, over AirPlay 2, Clementine also sends what's playing as an
iPhone does (see [APPLE_TRACES.md](APPLE_TRACES.md)): MediaRemote's
now-playing info in POST /command. Each song is a "replace" with its own
ContentItemIdentifier, an ArtworkIdentifier for its cover, and its rate,
position and a timestamp. "update"s then carry the position, the timestamp
and the rate. The session starts with the iPhone's empty info
and its list of supported commands. The DACP-ID is the same in every session.

| Check, 2026-11-01 | Result |
| --- | --- |
| The same, with a song's info replaced | A new cover for each song, over realtime audio. |
| Rate, position and timestamp in each song's replace | The clock stayed frozen. |
| The iPhone's updates, and its supported commands | The clock stayed frozen. |
| Now Playing's buttons, with supported commands sent | Each press came as a POST /command on the event channel, `sendMediaRemoteCommand` with its command (0 play, 4 next, 5 previous); Clementine doesn't act on them yet. Only Play is offered, as if paused. |

So the cover only needed the iPhone's fields. The clock is below.
Whether the steady DACP-ID stops the Allow prompt coming back needs
watching for longer.

### The clock, 2026-11-03

The Mac's own log (`log stream` over SSH) showed what runs it: MediaRemote's
playback state for the receiver's player, which is separate from the
playback rate in the now-playing info. The receiver sets it to stopped
whenever a session ends and otherwise leaves it to the sender. A rate of 1
only made it "playing" in the one session in which the state was still
unknown, the first after the Mac restarted, so that the clock seemed to run
by chance. An iPhone sets it with POST /command `updateMRPlaybackState`:
stopped once set up, playing before the audio, paused and playing again as
it pauses and resumes. It sends these, and `updateMRNowPlayingClient` and
`setMRInfo`, only to receivers with feature bit 60, which the Mac has and
shairport-sync hasn't, so they weren't in the traces from shairport-sync.

| Check | Result |
| --- | --- |
| `{type: updateMRPlaybackState, params: {mrPlaybackState: N}}` (1 playing, 2 paused, 3 stopped, as shairport-sync reads it), with NTP timing and realtime audio | The clock ran, and stopped at the pause. The Mac logged "Setting MR playback state to N" for each. |

So Clementine now sends it: stopped once the session is set up, then
playing or paused whenever that changes. PTP and buffered audio aren't
needed for the clock.

An update with a new timestamp and rate but no elapsed time, as an iPhone
sends one where the position is as the receiver would count it, took a
Mac's clock back to the elapsed time of the song's replace when Clementine
paused: it counts on from the last elapsed time it was sent, from the
latest timestamp. So every update says where it is.

### Other senders and buffered audio, 2026-11-01

| Check | Result |
| --- | --- |
| PipeWire's RAOP sink (AirPlay 1) | ANNOUNCE refused with 403, and it can't pair, so nothing played. |
| pyatv 0.18 | Realtime audio and SET_PARAMETER, as Clementine sends; it showed the title, no cover (it read none from the file) and a stopped clock. |
| OwnTone 29.3 | GET /info refused with 403: a Mac wants a User-Agent of AirPlay/ with a recent version (550.10 and 999.0.0 work, 29.3 and OwnTone/29.3 don't). |
| OwnTone's main branch, with its own PTP master on ports 319 and 320 | PTP with the Mac, realtime audio and SET_PARAMETER: it played and showed the first cover, but the clock stopped and the next song's cover didn't show. The same as Clementine. |
| Buffered audio (type 103, ALAC 48 kHz/24-bit) with NTP | The stream's SETUP refused with 400, also without the realtime stream's fields. The Mac lists the format in GET /info's supportedFormats. |
| The same, claiming PTP timing | SETUP accepted, with a TCP data port; the audio was taken, and a POST /command's title and cover were shown. SETRATEANCHORTIME, with an anchor not on a PTP clock, refused with 400, so nothing played. |

[APPLE_TRACES.md](APPLE_TRACES.md) has what an iPhone and a Mac send, decoded.

None of these runs a Mac's clock, as none sends `updateMRPlaybackState`
(see above).

## shairport-sync 4.3.7 (classic AirPlay)

On Linux, run in the foreground with its audio written to a file, tested
2026-11-01. It advertises only `_raop._tcp`, with `et=0,1`.

| Check | Result |
| --- | --- |
| Playing | The uncompressed ALAC decoded to the tone sent, 440 Hz then 660 Hz, with no gaps. |
| Volume | Taken as −3.10 dB for 70%. |
| Password | Advertised (`pw=true`), so asked for before connecting; a wrong one answered the Digest challenge (realm "raop") once and was refused; the right one played. |
| Now playing | Its metadata pipe gave each song's title, artist, album and persistent id, each song's cover (11.5 and 8.7 kB JPEGs) and progress. |

## shairport-sync 5.6.2 (AirPlay 2)

Built with AirPlay 2 support, run with nqptp in Docker on Linux, tested
2026-11-01. It advertises both `_airplay._tcp` and `_raop._tcp`.

| Check | Result |
| --- | --- |
| AirPlay 2 | Paired, then the session's SETUP refused with 501: it only takes PTP timing ("ntp stream handling is not implemented"). |
| Falling back to AirPlay 1 | Played: ANNOUNCE, SETUP, RECORD and FLUSH accepted, and "Play begin" in its log. |
