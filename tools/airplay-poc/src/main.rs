//! airplay-poc: plays audio on an AirPlay receiver.
//!
//!   airplay-poc list
//!   airplay-poc play <device name | host[:port]> [--tone | --wav FILE | -]
//!                    [--v1 | --v2] [--pin PIN] [--volume 0-100] [--seconds N]
//!
//! "-" reads raw S16LE 44.1 kHz stereo from stdin, e.g. from
//!   gst-launch-1.0 -q filesrc location=song.flac ! decodebin ! audioconvert !
//!     audioresample ! audio/x-raw,format=S16LE,rate=44100,channels=2 ! fdsink

mod discover;
mod hap;
mod raop;
mod rtsp;
mod srp;
mod tlv8;

use std::io::Read;
use std::net::IpAddr;
use std::time::Duration;

use anyhow::{anyhow, bail, Context, Result};
use log::info;

use raop::{Options, Session, Source, Version, SAMPLE_RATE};

struct Tone {
    frame: u64,
    limit: Option<u64>,
}

impl Source for Tone {
    fn read(&mut self, buf: &mut [u8]) -> Result<usize> {
        if self.limit.is_some_and(|l| self.frame >= l) {
            return Ok(0);
        }
        for f in buf.chunks_exact_mut(4) {
            // A 440 Hz tone that steps up a fifth every second, so gaps are audible.
            let t = self.frame as f64 / SAMPLE_RATE as f64;
            let freq = if (t as u64) % 2 == 0 { 440.0 } else { 660.0 };
            let s = ((t * freq * std::f64::consts::TAU).sin() * 8000.0) as i16;
            f[0..2].copy_from_slice(&s.to_le_bytes());
            f[2..4].copy_from_slice(&s.to_le_bytes());
            self.frame += 1;
        }
        Ok(buf.len())
    }
}

struct Reader<R: Read> {
    inner: R,
    limit: Option<u64>,
    read: u64,
}

impl<R: Read> Source for Reader<R> {
    fn read(&mut self, buf: &mut [u8]) -> Result<usize> {
        if self.limit.is_some_and(|l| self.read >= l * 4) {
            return Ok(0);
        }
        let mut n = 0;
        while n < buf.len() {
            let m = self.inner.read(&mut buf[n..])?;
            if m == 0 {
                break;
            }
            n += m;
        }
        self.read += n as u64;
        Ok(n)
    }
}

/// Skips a WAV header, checking it's what we send.
fn open_wav(path: &str) -> Result<std::io::BufReader<std::fs::File>> {
    let mut f = std::io::BufReader::new(std::fs::File::open(path).with_context(|| path.to_owned())?);
    let mut riff = [0u8; 12];
    f.read_exact(&mut riff)?;
    if &riff[0..4] != b"RIFF" || &riff[8..12] != b"WAVE" {
        bail!("{path} isn't a WAV file");
    }
    loop {
        let mut h = [0u8; 8];
        f.read_exact(&mut h)?;
        let len = u32::from_le_bytes([h[4], h[5], h[6], h[7]]) as usize;
        let mut body = vec![0u8; if &h[0..4] == b"data" { 0 } else { len + (len & 1) }];
        if &h[0..4] == b"data" {
            return Ok(f);
        }
        f.read_exact(&mut body)?;
        if &h[0..4] == b"fmt " {
            let ch = u16::from_le_bytes([body[2], body[3]]);
            let rate = u32::from_le_bytes([body[4], body[5], body[6], body[7]]);
            let bits = u16::from_le_bytes([body[14], body[15]]);
            if ch != 2 || rate != SAMPLE_RATE || bits != 16 {
                bail!("{path} is {ch} ch, {rate} Hz, {bits} bit; need 2 ch 44100 Hz 16 bit");
            }
        }
    }
}

fn main() -> Result<()> {
    env_logger::Builder::from_env(env_logger::Env::default().default_filter_or("info")).init();
    let args: Vec<String> = std::env::args().skip(1).collect();
    match args.first().map(String::as_str) {
        Some("list") => list(),
        Some("play") => play(&args[1..]),
        // Prints A, M1 and K for given a, salt, B and PIN, to check against srptools.
        Some("srp-test") => {
            let h = |s: &str| (0..s.len()).step_by(2).map(|i| u8::from_str_radix(&s[i..i + 2], 16).unwrap()).collect::<Vec<u8>>();
            let hex = |b: &[u8]| b.iter().map(|x| format!("{x:02x}")).collect::<String>();
            let s = srp::Client::with_private(&h(&args[1])).process(&args[4], &h(&args[2]), &h(&args[3]));
            println!("{}\n{}\n{}", hex(&s.public), hex(&s.proof), hex(&s.key));
            Ok(())
        }
        _ => {
            eprintln!("usage: airplay-poc list | play <device|host[:port]> [--tone|--wav FILE|-] [--v1|--v2] [--pin PIN] [--volume N] [--seconds N]");
            std::process::exit(2);
        }
    }
}

fn list() -> Result<()> {
    for d in discover::browse(Duration::from_secs(3))? {
        println!(
            "{}\n  addresses {:?}, raop port {:?}, airplay port {:?}, {:?}, features {:#x}",
            d.name,
            d.addresses,
            d.raop_port,
            d.airplay_port,
            d.version(),
            d.features()
        );
        let mut keys: Vec<_> = d.txt.iter().collect();
        keys.sort();
        for (k, v) in keys {
            println!("    {k}={v}");
        }
    }
    Ok(())
}

fn play(args: &[String]) -> Result<()> {
    let target = args.first().ok_or_else(|| anyhow!("play needs a device"))?;
    let mut version = None;
    let mut pin = "3939".to_owned();
    let mut volume = None;
    let mut seconds: Option<u64> = None;
    let mut input = "--tone".to_owned();
    let mut wav = None;
    let mut i = 1;
    while i < args.len() {
        match args[i].as_str() {
            "--v1" => version = Some(Version::AirPlay1),
            "--v2" => version = Some(Version::AirPlay2),
            "--pin" => {
                i += 1;
                pin = args.get(i).cloned().ok_or_else(|| anyhow!("--pin needs a value"))?;
            }
            "--volume" => {
                i += 1;
                volume = Some(args.get(i).ok_or_else(|| anyhow!("--volume needs a value"))?.parse()?);
            }
            "--seconds" => {
                i += 1;
                seconds = Some(args.get(i).ok_or_else(|| anyhow!("--seconds needs a value"))?.parse()?);
            }
            "--wav" => {
                i += 1;
                wav = Some(args.get(i).cloned().ok_or_else(|| anyhow!("--wav needs a file"))?);
                input = "--wav".into();
            }
            "--tone" | "-" => input = args[i].clone(),
            other => bail!("unknown option {other}"),
        }
        i += 1;
    }
    if let Ok(p) = std::env::var("AIRPLAY_PIN") {
        pin = p;
    }

    // A host or host:port, else a name to look up.
    let (host, port, detected) = if let Ok(ip) = target.parse::<IpAddr>() {
        (ip, 7000, None)
    } else if let Some((h, p)) = target.rsplit_once(':').and_then(|(h, p)| Some((h.parse::<IpAddr>().ok()?, p.parse::<u16>().ok()?))) {
        (h, p, None)
    } else {
        info!("Looking for {target}");
        let devices = discover::browse(Duration::from_secs(3))?;
        let d = devices
            .iter()
            .find(|d| d.name.eq_ignore_ascii_case(target))
            .or_else(|| devices.iter().find(|d| d.name.to_lowercase().contains(&target.to_lowercase())))
            .ok_or_else(|| anyhow!("no AirPlay device called {target}; try `airplay-poc list`"))?;
        info!("Found {} with features {:#x}", d.name, d.features());
        (
            d.address().ok_or_else(|| anyhow!("{} has no address", d.name))?,
            d.port().ok_or_else(|| anyhow!("{} has no port", d.name))?,
            Some(d.version()),
        )
    };
    let version = version.or(detected).unwrap_or(Version::AirPlay2);

    let limit = seconds.map(|s| s * SAMPLE_RATE as u64);
    let mut source: Box<dyn Source> = match input.as_str() {
        "--tone" => Box::new(Tone { frame: 0, limit: limit.or(Some(10 * SAMPLE_RATE as u64)) }),
        "--wav" => Box::new(Reader { inner: open_wav(wav.as_deref().unwrap())?, limit, read: 0 }),
        _ => Box::new(Reader { inner: std::io::stdin().lock(), limit, read: 0 }),
    };

    let mut session = Session::open(host, port, &Options { version, pin })?;
    session.play(source.as_mut(), volume)
}
