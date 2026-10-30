//! An AirPlay audio session: set-up over RTSP (AirPlay 1, or AirPlay 2 with
//! transient pairing), then uncompressed 44.1 kHz 16-bit stereo over RTP,
//! with the timing and control channels the receiver expects.

use std::collections::HashMap;
use std::io::{Read, Write};
use std::net::{IpAddr, SocketAddr, TcpStream, UdpSocket};
use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::{Arc, Mutex};
use std::thread;
use std::time::{Duration, Instant, SystemTime, UNIX_EPOCH};

use anyhow::{anyhow, bail, Context, Result};
use chacha20poly1305::aead::{Aead, KeyInit, Payload};
use chacha20poly1305::{ChaCha20Poly1305, Key, Nonce};
use log::{debug, info, warn};
use rand::Rng;

use crate::hap::{self, Cipher};
use crate::rtsp::{find, Connection};
use crate::srp;
use crate::tlv8;

pub const FRAMES_PER_PACKET: usize = 352;
pub const SAMPLE_RATE: u32 = 44100;
const FRAME_SIZE: usize = 4;
const BACKLOG: usize = 1000;
/// Receivers play this many frames behind the RTP timestamps (2.5 s).
const LATENCY: u32 = 22050 + SAMPLE_RATE;

#[derive(Clone, Copy, Debug, PartialEq)]
pub enum Version {
    AirPlay1,
    AirPlay2,
}

/// Where the audio comes from: interleaved S16LE stereo frames.
pub trait Source {
    /// Fills `buf` (a whole packet) and returns how many bytes it filled; 0 at the end.
    fn read(&mut self, buf: &mut [u8]) -> Result<usize>;
}

fn ntp_now() -> u64 {
    let d = SystemTime::now().duration_since(UNIX_EPOCH).unwrap();
    let secs = d.as_secs() + 0x83AA_7E80;
    let frac = ((d.subsec_micros() as u64) << 32) / 1_000_000;
    (secs << 32) | frac
}

fn ntp_to_ts(ntp: u64, rate: u32) -> u64 {
    ((ntp >> 16) * rate as u64) >> 16
}

fn ts_to_ntp(ts: u64, rate: u32) -> u64 {
    ((ts << 16) / rate as u64) << 16
}

struct Timeline {
    seq: u16,
    start_ts: u64,
    head_ts: u64,
}

impl Timeline {
    /// The RTP timestamp of the next packet: starts at the latency.
    fn rtptime(&self) -> u32 {
        (self.head_ts - self.start_ts + LATENCY as u64) as u32
    }
}

struct Shared {
    timeline: Mutex<Timeline>,
    backlog: Mutex<HashMap<u16, Vec<u8>>>,
    running: Arc<AtomicBool>,
}

pub struct Session {
    conn: Arc<Mutex<Connection>>,
    version: Version,
    shared: Arc<Shared>,
    control: UdpSocket,
    audio: UdpSocket,
    remote_control: SocketAddr,
    rtsp_session: String,
    /// AirPlay 2 encrypts each audio packet with the key it sent in SETUP.
    audio_cipher: Option<ChaCha20Poly1305>,
    _event_channel: Option<TcpStream>,
}

pub struct Options {
    pub version: Version,
    pub pin: String,
}

impl Session {
    pub fn open(host: IpAddr, port: u16, opts: &Options) -> Result<Session> {
        let mut conn = Connection::connect(host, port)?;
        info!("Connected to {host}:{port} from {}, using {:?}", conn.local_ip, opts.version);

        let control = UdpSocket::bind((conn.local_ip, 0))?;
        let timing = UdpSocket::bind((conn.local_ip, 0))?;
        let control_port = control.local_addr()?.port();
        let timing_port = timing.local_addr()?.port();
        // Receivers ask for the time while they set up, and may not answer
        // SETUP until we've answered them, so this starts first.
        let running = Arc::new(AtomicBool::new(true));
        spawn_timing(timing.try_clone()?, running.clone());

        let info = conn.request("GET", Some("/info"), &[], None, &[], "RTSP/1.0")?;
        if info.code == 200 {
            if let Ok(v) = plist::Value::from_reader(std::io::Cursor::new(&info.body)) {
                let dict = v.as_dictionary();
                let get = |k: &str| {
                    dict.and_then(|d| d.get(k))
                        .and_then(|v| v.as_string())
                        .unwrap_or("?")
                        .to_owned()
                };
                info!("Device: {} ({}), source version {}", get("name"), get("model"), get("sourceVersion"));
            }
        }

        let mut rtsp_session = String::new();
        let mut audio_cipher = None;
        let mut event_channel = None;
        let (data_port, remote_control_port);

        match opts.version {
            Version::AirPlay1 => {
                let sdp = format!(
                    "v=0\r\no=iTunes {sid} 0 IN IP4 {local}\r\ns=iTunes\r\nc=IN IP4 {remote}\r\nt=0 0\r\n\
                     m=audio 0 RTP/AVP 96\r\na=rtpmap:96 AppleLossless\r\n\
                     a=fmtp:96 {FRAMES_PER_PACKET} 0 16 40 10 14 2 255 0 0 44100\r\n",
                    sid = conn.session_id,
                    local = conn.local_ip,
                    remote = conn.remote_ip,
                );
                conn.rtsp_ok("ANNOUNCE", &[], Some("application/sdp"), sdp.as_bytes())?;
                let resp = conn.rtsp_ok(
                    "SETUP",
                    &[(
                        "Transport",
                        format!(
                            "RTP/AVP/UDP;unicast;interleaved=0-1;mode=record;\
                             control_port={control_port};timing_port={timing_port}"
                        ),
                    )],
                    None,
                    &[],
                )?;
                let transport = resp.header("Transport").ok_or_else(|| anyhow!("no Transport in SETUP"))?;
                let opt = |k: &str| -> Option<u16> {
                    transport
                        .split(';')
                        .filter_map(|o| o.split_once('='))
                        .find(|(key, _)| *key == k)
                        .and_then(|(_, v)| v.parse().ok())
                };
                data_port = opt("server_port").ok_or_else(|| anyhow!("no server_port"))?;
                remote_control_port = opt("control_port").ok_or_else(|| anyhow!("no control_port"))?;
                rtsp_session = resp.header("Session").unwrap_or("1").to_owned();
            }
            Version::AirPlay2 => {
                let shared_key = transient_pair(&mut conn, &opts.pin)?;
                let out = hap::derive("Control-Salt", "Control-Write-Encryption-Key", &shared_key);
                let inp = hap::derive("Control-Salt", "Control-Read-Encryption-Key", &shared_key);
                conn.enable_encryption(Cipher::new(&out, &inp));
                info!("Paired (transient), connection encrypted");

                let mut base = plist::Dictionary::new();
                base.insert("deviceID".into(), "AA:BB:CC:DD:EE:FF".into());
                base.insert("macAddress".into(), "AA:BB:CC:DD:EE:FF".into());
                base.insert("sessionUUID".into(), uuid().into());
                base.insert("timingPort".into(), (timing_port as u64).into());
                base.insert("timingProtocol".into(), "NTP".into());
                base.insert("isMultiSelectAirPlay".into(), true.into());
                base.insert("groupContainsGroupLeader".into(), false.into());
                base.insert("model".into(), "iPhone14,3".into());
                base.insert("name".into(), "Clementine".into());
                base.insert("osBuildVersion".into(), "20F66".into());
                base.insert("osName".into(), "iPhone OS".into());
                base.insert("osVersion".into(), "16.5".into());
                base.insert("senderSupportsRelay".into(), false.into());
                base.insert("sourceVersion".into(), "690.7.1".into());
                base.insert("statsCollectionEnabled".into(), false.into());
                let resp = conn.rtsp_ok("SETUP", &[], Some(BPLIST), &bplist(base)?)?;
                let resp = parse_plist(&resp.body)?;
                let event_port = resp
                    .get("eventPort")
                    .and_then(|v| v.as_unsigned_integer())
                    .unwrap_or(0) as u16;
                debug!("Event port {event_port}");

                if event_port != 0 {
                    let out = hap::derive("Events-Salt", "Events-Read-Encryption-Key", &shared_key);
                    let inp = hap::derive("Events-Salt", "Events-Write-Encryption-Key", &shared_key);
                    event_channel = Some(open_event_channel(host, event_port, Cipher::new(&out, &inp))?);
                }

                let shk = hap::derive("Events-Salt", "Events-Write-Encryption-Key", &shared_key);
                let mut stream = plist::Dictionary::new();
                stream.insert("audioFormat".into(), 0x800u64.into());
                stream.insert("audioMode".into(), "default".into());
                stream.insert("controlPort".into(), (control_port as u64).into());
                stream.insert("ct".into(), 1u64.into());
                stream.insert("isMedia".into(), true.into());
                stream.insert("latencyMax".into(), 88200u64.into());
                stream.insert("latencyMin".into(), 11025u64.into());
                stream.insert("shk".into(), plist::Value::Data(shk.to_vec()));
                stream.insert("spf".into(), (FRAMES_PER_PACKET as u64).into());
                stream.insert("sr".into(), (SAMPLE_RATE as u64).into());
                stream.insert("type".into(), 0x60u64.into());
                stream.insert("supportsDynamicStreamID".into(), false.into());
                stream.insert("streamConnectionID".into(), (conn.session_id as u64).into());
                let mut body = plist::Dictionary::new();
                body.insert("streams".into(), plist::Value::Array(vec![stream.into()]));
                let resp = conn.rtsp_ok("SETUP", &[], Some(BPLIST), &bplist(body)?)?;
                let resp = parse_plist(&resp.body)?;
                let s = resp
                    .get("streams")
                    .and_then(|v| v.as_array())
                    .and_then(|a| a.first())
                    .and_then(|v| v.as_dictionary())
                    .ok_or_else(|| anyhow!("no streams in SETUP response"))?;
                let port = |k: &str| s.get(k).and_then(|v| v.as_unsigned_integer()).map(|p| p as u16);
                data_port = port("dataPort").ok_or_else(|| anyhow!("no dataPort"))?;
                remote_control_port = port("controlPort").ok_or_else(|| anyhow!("no controlPort"))?;
                audio_cipher = Some(ChaCha20Poly1305::new(Key::from_slice(&shk)));
            }
        }
        info!("Receiver ports: audio {data_port}, control {remote_control_port}");

        let audio = UdpSocket::bind((conn.local_ip, 0))?;
        audio.connect((host, data_port))?;

        let start_ts = ntp_to_ts(ntp_now(), SAMPLE_RATE);
        let shared = Arc::new(Shared {
            timeline: Mutex::new(Timeline { seq: rand::thread_rng().gen(), start_ts, head_ts: start_ts }),
            backlog: Mutex::new(HashMap::new()),
            running,
        });

        Ok(Session {
            conn: Arc::new(Mutex::new(conn)),
            version: opts.version,
            shared,
            control,
            audio,
            remote_control: (host, remote_control_port).into(),
            rtsp_session,
            audio_cipher,
            _event_channel: event_channel,
        })
    }

    fn session_headers(&self) -> Vec<(&'static str, String)> {
        if self.rtsp_session.is_empty() {
            vec![]
        } else {
            vec![("Session", self.rtsp_session.clone())]
        }
    }

    /// Volume from 0 to 100, which AirPlay takes as -30 to 0 dB.
    pub fn set_volume(&self, percent: f32) -> Result<()> {
        let db = if percent <= 0.0 { -144.0 } else { -30.0 + 30.0 * percent.min(100.0) / 100.0 };
        let mut conn = self.conn.lock().unwrap();
        let resp = conn.rtsp(
            "SET_PARAMETER",
            &self.session_headers(),
            Some("text/parameters"),
            format!("volume: {db:.6}\r\n").as_bytes(),
        )?;
        if resp.code != 200 {
            warn!("Setting the volume failed: {}", resp.code);
        }
        Ok(())
    }

    pub fn play(&mut self, source: &mut dyn Source, volume: Option<f32>) -> Result<()> {
        self.spawn_control()?;
        self.spawn_feedback();

        let (seq, rtptime) = {
            let t = self.shared.timeline.lock().unwrap();
            (t.seq, t.rtptime())
        };
        {
            let mut conn = self.conn.lock().unwrap();
            conn.rtsp_ok("RECORD", &self.session_headers(), None, &[])?;
            let mut h = self.session_headers();
            h.push(("Range", "npt=0-".into()));
            h.push(("RTP-Info", format!("seq={seq};rtptime={rtptime}")));
            let resp = conn.rtsp("FLUSH", &h, None, &[])?;
            debug!("FLUSH: {}", resp.code);
        }
        if let Some(v) = volume {
            self.set_volume(v)?;
        }
        info!("Streaming");

        let result = self.stream(source);
        self.shared.running.store(false, Ordering::SeqCst);
        let mut conn = self.conn.lock().unwrap();
        let _ = conn.rtsp("TEARDOWN", &self.session_headers(), None, &[]);
        result
    }

    fn stream(&mut self, source: &mut dyn Source) -> Result<()> {
        let started = Instant::now();
        let mut sent_frames: u64 = 0;
        let mut padding: u64 = 0;
        let mut first = true;
        let mut nonce_counter: u64 = 0;
        let mut pcm = vec![0u8; FRAMES_PER_PACKET * FRAME_SIZE];
        let mut last_report = Instant::now();

        // Once the source ends, keep sending silence for the latency so what
        // was sent gets played out.
        while padding < LATENCY as u64 {
            let n = source.read(&mut pcm)?;
            if n == 0 {
                pcm.fill(0);
                padding += FRAMES_PER_PACKET as u64;
            } else {
                pcm[n..].fill(0);
            }
            let payload = match self.version {
                // AirPlay 1 receivers all take ALAC, so wrap the PCM in
                // uncompressed ALAC frames.
                Version::AirPlay1 => alac_uncompressed(&pcm),
                // AirPlay 2 takes L16, which is big endian.
                Version::AirPlay2 => pcm.chunks_exact(2).flat_map(|s| [s[1], s[0]]).collect(),
            };

            let (seq, rtptime) = {
                let t = self.shared.timeline.lock().unwrap();
                (t.seq, t.rtptime())
            };
            let mut packet = Vec::with_capacity(12 + pcm.len() + 24);
            packet.push(0x80);
            packet.push(if first { 0xE0 } else { 0x60 });
            packet.extend_from_slice(&seq.to_be_bytes());
            packet.extend_from_slice(&rtptime.to_be_bytes());
            packet.extend_from_slice(&self.conn.lock().unwrap().session_id.to_be_bytes());
            match &self.audio_cipher {
                Some(c) => {
                    let nonce = hap::nonce(nonce_counter);
                    nonce_counter += 1;
                    let aad = packet[4..12].to_vec();
                    let ct = c
                        .encrypt(Nonce::from_slice(&nonce), Payload { msg: &payload, aad: &aad })
                        .map_err(|_| anyhow!("encrypting audio"))?;
                    packet.extend_from_slice(&ct);
                    packet.extend_from_slice(&nonce[4..]);
                }
                None => packet.extend_from_slice(&payload),
            }
            first = false;

            if let Err(e) = self.audio.send(&packet) {
                warn!("Sending audio: {e}");
            }
            {
                let mut backlog = self.shared.backlog.lock().unwrap();
                backlog.insert(seq, packet);
                backlog.remove(&seq.wrapping_sub(BACKLOG as u16));
            }
            {
                let mut t = self.shared.timeline.lock().unwrap();
                t.seq = t.seq.wrapping_add(1);
                t.head_ts += FRAMES_PER_PACKET as u64;
            }
            sent_frames += FRAMES_PER_PACKET as u64;

            if last_report.elapsed() >= Duration::from_secs(5) {
                info!("Sent {:.1} s of audio", sent_frames as f64 / SAMPLE_RATE as f64);
                last_report = Instant::now();
            }

            // Pace to real time: sleep until this packet is due.
            let due = Duration::from_secs_f64(sent_frames as f64 / SAMPLE_RATE as f64);
            let elapsed = started.elapsed();
            if due > elapsed {
                thread::sleep(due - elapsed);
            }
        }
        info!("Finished after {:.1} s", started.elapsed().as_secs_f64());
        Ok(())
    }

    /// Sends a sync packet every second and answers resend requests.
    fn spawn_control(&self) -> Result<()> {
        let sock = self.control.try_clone()?;
        sock.set_read_timeout(Some(Duration::from_millis(200)))?;
        let shared = self.shared.clone();
        let dest = self.remote_control;
        thread::spawn(move || {
            let mut first = true;
            let mut last_sync = Instant::now() - Duration::from_secs(2);
            let mut buf = [0u8; 2048];
            while shared.running.load(Ordering::SeqCst) {
                if last_sync.elapsed() >= Duration::from_secs(1) {
                    let (rtptime, head) = {
                        let t = shared.timeline.lock().unwrap();
                        (t.rtptime(), t.head_ts)
                    };
                    let ntp = ts_to_ntp(head, SAMPLE_RATE);
                    let mut p = Vec::with_capacity(20);
                    p.push(if first { 0x90 } else { 0x80 });
                    p.push(0xD4);
                    p.extend_from_slice(&7u16.to_be_bytes());
                    p.extend_from_slice(&rtptime.wrapping_sub(LATENCY).to_be_bytes());
                    p.extend_from_slice(&ntp.to_be_bytes());
                    p.extend_from_slice(&rtptime.to_be_bytes());
                    let _ = sock.send_to(&p, dest);
                    first = false;
                    last_sync = Instant::now();
                }
                let Ok((n, from)) = sock.recv_from(&mut buf) else { continue };
                if n >= 8 && buf[1] & 0x7F == 0x55 {
                    let lost = u16::from_be_bytes([buf[4], buf[5]]);
                    let count = u16::from_be_bytes([buf[6], buf[7]]);
                    debug!("Resend request for {count} packets from {lost}");
                    let backlog = shared.backlog.lock().unwrap();
                    for i in 0..count {
                        let seq = lost.wrapping_add(i);
                        if let Some(packet) = backlog.get(&seq) {
                            let mut r = vec![0x80, 0xD6];
                            r.extend_from_slice(&packet[2..4]);
                            r.extend_from_slice(packet);
                            let _ = sock.send_to(&r, from);
                        }
                    }
                } else {
                    debug!("Control packet type {:#x}", buf[1] & 0x7F);
                }
            }
        });
        Ok(())
    }

    /// Keeps the session alive with POST /feedback, which AirPlay 2 needs.
    fn spawn_feedback(&self) {
        let conn = self.conn.clone();
        let shared = self.shared.clone();
        let interval = match self.version {
            Version::AirPlay1 => Duration::from_secs(25),
            Version::AirPlay2 => Duration::from_secs(2),
        };
        thread::spawn(move || {
            let mut last = Instant::now();
            while shared.running.load(Ordering::SeqCst) {
                thread::sleep(Duration::from_millis(100));
                if last.elapsed() < interval {
                    continue;
                }
                last = Instant::now();
                let mut c = conn.lock().unwrap();
                match c.request("POST", Some("/feedback"), &[], None, &[], "RTSP/1.0") {
                    Ok(r) if r.code != 200 => debug!("feedback: {}", r.code),
                    Ok(_) => {}
                    Err(e) => debug!("feedback failed: {e}"),
                }
            }
        });
    }
}

/// An ALAC frame holding PCM as it is (the "escape" frame every decoder
/// handles): a stereo element with the escape flag, the samples as 16-bit
/// big-endian, then the end tag.
fn alac_uncompressed(pcm: &[u8]) -> Vec<u8> {
    let mut w = BitWriter::default();
    w.put(1, 3); // ID_CPE: a channel pair
    w.put(0, 4); // element instance tag
    w.put(0, 12); // unused
    w.put(0b0001, 4); // not partial, no shift, escape (uncompressed)
    for s in pcm.chunks_exact(2) {
        w.put(u16::from_le_bytes([s[0], s[1]]) as u32, 16);
    }
    w.put(7, 3); // ID_END
    w.finish()
}

#[derive(Default)]
struct BitWriter {
    out: Vec<u8>,
    acc: u64,
    bits: u32,
}

impl BitWriter {
    fn put(&mut self, value: u32, n: u32) {
        self.acc = (self.acc << n) | (value as u64 & ((1u64 << n) - 1));
        self.bits += n;
        while self.bits >= 8 {
            self.bits -= 8;
            self.out.push((self.acc >> self.bits) as u8);
        }
    }

    fn finish(mut self) -> Vec<u8> {
        if self.bits > 0 {
            self.out.push((self.acc << (8 - self.bits)) as u8);
        }
        self.out
    }
}

/// Answers the receiver's NTP-style timing requests.
fn spawn_timing(sock: UdpSocket, running: Arc<AtomicBool>) {
    sock.set_read_timeout(Some(Duration::from_millis(500))).unwrap();
    thread::spawn(move || {
        let mut buf = [0u8; 128];
        while running.load(Ordering::SeqCst) {
            let Ok((n, from)) = sock.recv_from(&mut buf) else { continue };
            debug!("Timing request from {from}");
            if n < 32 {
                continue;
            }
            let now = ntp_now();
            let mut resp = [0u8; 32];
            resp[0] = buf[0];
            resp[1] = 0x53 | 0x80;
            resp[2..4].copy_from_slice(&7u16.to_be_bytes());
            // Reference time: when they sent the request.
            resp[8..16].copy_from_slice(&buf[24..32]);
            resp[16..24].copy_from_slice(&now.to_be_bytes());
            resp[24..32].copy_from_slice(&now.to_be_bytes());
            let _ = sock.send_to(&resp, from);
        }
    });
}

const BPLIST: &str = "application/x-apple-binary-plist";

fn bplist(d: plist::Dictionary) -> Result<Vec<u8>> {
    let mut out = Vec::new();
    plist::Value::Dictionary(d).to_writer_binary(&mut out)?;
    Ok(out)
}

fn parse_plist(body: &[u8]) -> Result<plist::Dictionary> {
    let v = plist::Value::from_reader(std::io::Cursor::new(body)).context("parsing plist")?;
    v.into_dictionary().ok_or_else(|| anyhow!("plist isn't a dictionary"))
}

fn uuid() -> String {
    let b: [u8; 16] = rand::thread_rng().gen();
    let h: String = b.iter().map(|x| format!("{x:02X}")).collect();
    format!("{}-{}-{}-{}-{}", &h[0..8], &h[8..12], &h[12..16], &h[16..20], &h[20..32])
}

/// HomeKit transient pairing (pair-setup M1 to M4 with a fixed or user PIN),
/// returning the SRP shared key the session keys derive from.
fn transient_pair(conn: &mut Connection, pin: &str) -> Result<Vec<u8>> {
    let headers = [("X-Apple-HKP", "4".to_owned()), ("Connection", "keep-alive".to_owned())];
    let r = conn.post("/pair-pin-start", &headers, &[])?;
    debug!("pair-pin-start: {}", r.code);

    let m1 = tlv8::write(&[
        (tlv8::METHOD, &[0]),
        (tlv8::SEQ_NO, &[1]),
        (tlv8::FLAGS, &[tlv8::FLAG_TRANSIENT]),
    ]);
    let r = conn.post("/pair-setup", &headers, &m1)?;
    if r.code != 200 {
        bail!("pair-setup M1 refused: {} (is \"Allow AirPlay for\" set to anyone on the network?)", r.code);
    }
    let m2 = tlv8::read(&r.body);
    if let Some(e) = tlv8::get(&m2, tlv8::ERROR) {
        bail!("pair-setup M2 error {e:?}");
    }
    let salt = tlv8::get(&m2, tlv8::SALT).ok_or_else(|| anyhow!("no salt in M2"))?;
    let b_pub = tlv8::get(&m2, tlv8::PUBLIC_KEY).ok_or_else(|| anyhow!("no public key in M2"))?;

    let client = srp::Client::new();
    let session = client.process(pin, salt, b_pub);
    let m3 = tlv8::write(&[
        (tlv8::SEQ_NO, &[3]),
        (tlv8::PUBLIC_KEY, &session.public),
        (tlv8::PROOF, &session.proof),
    ]);
    let r = conn.post("/pair-setup", &headers, &m3)?;
    let m4 = tlv8::read(&r.body);
    if r.code != 200 || tlv8::get(&m4, tlv8::ERROR).is_some() {
        bail!(
            "pair-setup M4 refused ({}, error {:?}): wrong PIN or password?",
            r.code,
            tlv8::get(&m4, tlv8::ERROR)
        );
    }
    Ok(session.key)
}

/// The receiver sends RTSP requests to us on the event channel; we answer
/// each with 200 OK, as Apple's senders do.
fn open_event_channel(host: IpAddr, port: u16, mut cipher: Cipher) -> Result<TcpStream> {
    let mut stream = None;
    for _ in 0..5 {
        match TcpStream::connect((host, port)) {
            Ok(s) => {
                stream = Some(s);
                break;
            }
            Err(e) => {
                debug!("event channel: {e}, retrying");
                thread::sleep(Duration::from_secs(1));
            }
        }
    }
    let stream = stream.ok_or_else(|| anyhow!("couldn't open the event channel"))?;
    let mut reader = stream.try_clone()?;
    let mut writer = stream.try_clone()?;
    thread::spawn(move || {
        let mut buf = [0u8; 4096];
        let mut plain = Vec::new();
        loop {
            let n = match reader.read(&mut buf) {
                Ok(0) | Err(_) => break,
                Ok(n) => n,
            };
            match cipher.decrypt(&buf[..n]) {
                Ok(p) => plain.extend_from_slice(&p),
                Err(e) => {
                    warn!("event channel: {e}");
                    break;
                }
            }
            while let Some(end) = find(&plain, b"\r\n\r\n") {
                let head = String::from_utf8_lossy(&plain[..end]).into_owned();
                let len: usize = head
                    .lines()
                    .filter_map(|l| l.split_once(':'))
                    .find(|(k, _)| k.trim().eq_ignore_ascii_case("Content-Length"))
                    .and_then(|(_, v)| v.trim().parse().ok())
                    .unwrap_or(0);
                if plain.len() < end + 4 + len {
                    break;
                }
                plain.drain(..end + 4 + len);
                debug!("event: {}", head.lines().next().unwrap_or(""));
                let cseq = head
                    .lines()
                    .filter_map(|l| l.split_once(':'))
                    .find(|(k, _)| k.trim().eq_ignore_ascii_case("CSeq"))
                    .map(|(_, v)| v.trim().to_owned())
                    .unwrap_or_default();
                let resp = format!(
                    "RTSP/1.0 200 OK\r\nContent-Length: 0\r\nAudio-Latency: 0\r\nCSeq: {cseq}\r\n\r\n"
                );
                if writer.write_all(&cipher.encrypt(resp.as_bytes())).is_err() {
                    return;
                }
            }
        }
    });
    Ok(stream)
}
