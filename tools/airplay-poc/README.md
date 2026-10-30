# airplay-poc

A proof of concept for AirPlay output: a small Rust program that streams
audio to an AirPlay receiver. It exists to try out the protocol before
designing a GStreamer sink for Clementine, and isn't built with Clementine.

It speaks:

- **AirPlay 1 (RAOP):** ANNOUNCE / SETUP / RECORD over RTSP, then RTP audio as
  uncompressed ALAC frames, which every AirPlay 1 receiver decodes.
- **AirPlay 2:** HomeKit transient pairing (SRP with PIN 3939, or a password),
  an encrypted RTSP connection and event channel, and ChaCha20-Poly1305
  encrypted RTP audio (L16), with NTP timing.

Both send sync packets on the control channel, answer resend requests and
timing requests, and pace the audio to real time.

The protocol details follow [pyatv](https://github.com/postlund/pyatv)
(MIT), which is the clearest reference for senders.

## Using it

```sh
cargo build --release
target/release/airplay-poc list
target/release/airplay-poc play "Living Room" --tone --seconds 10
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
    target/release/airplay-poc play "Living Room" -
  ```

Other options: `--v1` or `--v2` to override the AirPlay version chosen from
the device's mDNS features, `--volume 0-100`, `--seconds N`, and `--pin PIN`
for a device with a password (or set `AIRPLAY_PIN`, to keep it out of shell
history). `RUST_LOG=debug` logs every RTSP request and response.

A Mac can be the receiver: turn on System Settings > General > AirDrop &
Handoff > AirPlay Receiver, and set "Allow AirPlay for" to anyone on the same
network, since only Apple devices can sign in to "Current User".

shairport-sync works as an AirPlay 1 test receiver:

```sh
shairport-sync -o stdout -p 5000 > out.raw &
target/release/airplay-poc play 127.0.0.1:5000 --v1 --tone --seconds 4
```
