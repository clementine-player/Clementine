# airplay-poc

A proof of concept for AirPlay output: a small Qt program that streams audio
to an AirPlay receiver. It tries out the protocol in the language and with the
libraries Clementine would use, before the session and a GStreamer sink are
built into Clementine. It isn't built with Clementine.

It speaks:

- **AirPlay 1 (RAOP):** ANNOUNCE / SETUP / RECORD over RTSP, then RTP audio as
  uncompressed ALAC frames, which every AirPlay 1 receiver decodes.
- **AirPlay 2:** HomeKit transient pairing (SRP with PIN 3939, or a password),
  an encrypted RTSP connection and event channel, and ChaCha20-Poly1305
  encrypted RTP audio (L16), with NTP timing.

Both send sync packets on the control channel, answer resend requests and
timing requests, and pace the audio to real time. Everything runs on the Qt
event loop: receivers ask for the time while they set up, and may not answer
SETUP until they've had an answer.

The protocol details follow [pyatv](https://github.com/postlund/pyatv) (MIT),
the clearest reference for senders.

## Libraries

- Qt 6 (Core, Network, DBus, Test): sockets, SHA-512, HMAC (for HKDF) and
  random numbers.
- [Monocypher](../../3rdparty/monocypher) for ChaCha20-Poly1305.
- [mini-gmp](../../3rdparty/mini-gmp) for the big-number arithmetic of SRP.
- avahi-daemon, over D-Bus, to find receivers. Linux only.

## Using it

```sh
cmake -S tools/airplay-poc-cpp -B build-airplay -G Ninja
ninja -C build-airplay
build-airplay/airplay-poc-tests
build-airplay/airplay-poc list
build-airplay/airplay-poc play "Living Room" --tone --seconds 10
```

`play` takes a device name from `list` (or part of one), or an address with
an optional port. The audio is one of:

- `--tone`: a test tone that alternates between 440 and 660 Hz every second
  (the default, for 10 seconds).
- `--wav FILE`: a 16-bit 44.1 kHz stereo WAV file.
- `-`: raw S16LE 44.1 kHz stereo on stdin, for example from GStreamer:

  ```sh
  gst-launch-1.0 -q filesrc location=song.flac ! decodebin ! audioconvert ! \
    audioresample ! audio/x-raw,format=S16LE,rate=44100,channels=2 ! fdsink | \
    build-airplay/airplay-poc play "Living Room" -
  ```

Other options: `--v1` or `--v2` to override the AirPlay version chosen from
the device's mDNS features, `--volume 0-100`, `--seconds N`, `--name NAME` for
what the receiver calls this sender (by default "Clementine on <hostname>",
which a Mac shows when it asks whether to allow it), `--device-id HEX` for a
fixed sender id (random for each run by default), and `--pin PIN`
for a device with a password (or set `AIRPLAY_PIN`, to keep it out of shell
history). `AIRPLAY_DEBUG=1` logs every RTSP request and response.

A Mac can be the receiver: turn on System Settings > General > AirDrop &
Handoff > AirPlay Receiver, and set "Allow AirPlay for" to anyone on the same
network, since only Apple devices can sign in to "Current User".

shairport-sync works as an AirPlay 1 test receiver:

```sh
shairport-sync -o stdout -p 5000 > out.raw &
build-airplay/airplay-poc play 127.0.0.1:5000 --v1 --tone --seconds 4
```
