//! TLV8, the HomeKit pairing message format. Values over 255 bytes are split
//! into consecutive items of the same type.

pub const METHOD: u8 = 0x00;
pub const SALT: u8 = 0x02;
pub const PUBLIC_KEY: u8 = 0x03;
pub const PROOF: u8 = 0x04;
pub const SEQ_NO: u8 = 0x06;
pub const ERROR: u8 = 0x07;
pub const FLAGS: u8 = 0x13;

pub const FLAG_TRANSIENT: u8 = 0x10;

pub fn write(items: &[(u8, &[u8])]) -> Vec<u8> {
    let mut out = Vec::new();
    for (t, v) in items {
        if v.is_empty() {
            out.extend_from_slice(&[*t, 0]);
        }
        for chunk in v.chunks(255) {
            out.push(*t);
            out.push(chunk.len() as u8);
            out.extend_from_slice(chunk);
        }
    }
    out
}

pub fn read(data: &[u8]) -> Vec<(u8, Vec<u8>)> {
    let mut out: Vec<(u8, Vec<u8>)> = Vec::new();
    let mut i = 0;
    let mut last: Option<u8> = None;
    while i + 2 <= data.len() {
        let t = data[i];
        let len = data[i + 1] as usize;
        let v = &data[i + 2..(i + 2 + len).min(data.len())];
        match out.last_mut() {
            Some((lt, lv)) if last == Some(t) && *lt == t => lv.extend_from_slice(v),
            _ => out.push((t, v.to_vec())),
        }
        last = Some(t);
        i += 2 + len;
    }
    out
}

pub fn get(items: &[(u8, Vec<u8>)], t: u8) -> Option<&[u8]> {
    items.iter().find(|(it, _)| *it == t).map(|(_, v)| v.as_slice())
}
