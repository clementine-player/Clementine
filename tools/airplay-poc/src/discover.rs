//! Finding AirPlay receivers with mDNS (_raop._tcp and _airplay._tcp).

use std::collections::HashMap;
use std::net::IpAddr;
use std::time::{Duration, Instant};

use anyhow::Result;
use mdns_sd::{ServiceDaemon, ServiceEvent};

use crate::raop::Version;

#[derive(Clone, Debug)]
pub struct Device {
    pub name: String,
    pub addresses: Vec<IpAddr>,
    pub raop_port: Option<u16>,
    pub airplay_port: Option<u16>,
    pub txt: HashMap<String, String>,
}

impl Device {
    pub fn features(&self) -> u64 {
        let f = self.txt.get("ft").or_else(|| self.txt.get("features"));
        let Some(f) = f else { return 0 };
        let mut parts = f.split(',').map(|p| u64::from_str_radix(p.trim_start_matches("0x"), 16).unwrap_or(0));
        let low = parts.next().unwrap_or(0);
        let high = parts.next().unwrap_or(0);
        (high << 32) | low
    }

    /// AirPlay 2 if it does HomeKit-style pairing, as pyatv decides.
    pub fn version(&self) -> Version {
        let f = self.features();
        if f & (1 << 38) != 0 || f & (1 << 48) != 0 {
            Version::AirPlay2
        } else {
            Version::AirPlay1
        }
    }

    pub fn port(&self) -> Option<u16> {
        self.raop_port.or(self.airplay_port)
    }

    pub fn address(&self) -> Option<IpAddr> {
        self.addresses.iter().find(|a| a.is_ipv4()).or(self.addresses.first()).copied()
    }
}

pub fn browse(wait: Duration) -> Result<Vec<Device>> {
    let mdns = ServiceDaemon::new()?;
    let raop = mdns.browse("_raop._tcp.local.")?;
    let airplay = mdns.browse("_airplay._tcp.local.")?;
    let mut devices: HashMap<String, Device> = HashMap::new();
    let deadline = Instant::now() + wait;
    while Instant::now() < deadline {
        for (rx, is_raop) in [(&raop, true), (&airplay, false)] {
            while let Ok(ev) = rx.try_recv() {
                let ServiceEvent::ServiceResolved(info) = ev else { continue };
                let instance = info.get_fullname().split("._").next().unwrap_or("").to_owned();
                // RAOP names are "<MAC>@<name>".
                let name = instance.split_once('@').map(|(_, n)| n.to_owned()).unwrap_or(instance);
                let d = devices.entry(name.clone()).or_insert_with(|| Device {
                    name,
                    addresses: vec![],
                    raop_port: None,
                    airplay_port: None,
                    txt: HashMap::new(),
                });
                for a in info.get_addresses() {
                    if !d.addresses.contains(a) {
                        d.addresses.push(*a);
                    }
                }
                if is_raop {
                    d.raop_port = Some(info.get_port());
                } else {
                    d.airplay_port = Some(info.get_port());
                }
                for p in info.get_properties().iter() {
                    d.txt.entry(p.key().to_owned()).or_insert_with(|| p.val_str().to_owned());
                }
            }
        }
        std::thread::sleep(Duration::from_millis(50));
    }
    let _ = mdns.shutdown();
    let mut v: Vec<Device> = devices.into_values().collect();
    v.sort_by(|a, b| a.name.cmp(&b.name));
    Ok(v)
}
