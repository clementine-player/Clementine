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

So over realtime audio the Mac shows what's playing and applies seeks, but
shows it as paused and keeps the first song's cover. It may want buffered
audio (type 103) with SETRATEANCHORTIME, or MediaRemote, for those, which
aren't implemented.

## shairport-sync 4.3.7 (classic AirPlay)

On Linux, run in the foreground with its audio written to a file, tested
2026-11-01. It advertises only `_raop._tcp`, with `et=0,1`.

| Check | Result |
| --- | --- |
| Playing | The uncompressed ALAC decoded to the tone sent, 440 Hz then 660 Hz, with no gaps. |
| Volume | Taken as −3.10 dB for 70%. |
| Password | Advertised (`pw=true`), so asked for before connecting; a wrong one answered the Digest challenge (realm "raop") once and was refused; the right one played. |
| Now playing | Its metadata pipe gave each song's title, artist, album and persistent id, each song's cover (11.5 and 8.7 kB JPEGs) and progress. |
