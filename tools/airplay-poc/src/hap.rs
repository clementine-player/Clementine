//! The encryption AirPlay 2 puts on its TCP connections after pairing:
//! ChaCha20-Poly1305 in frames of at most 1024 bytes, each preceded by its
//! length (little endian), which is also the additional authenticated data.

use anyhow::{anyhow, Result};
use chacha20poly1305::aead::{Aead, KeyInit, Payload};
use chacha20poly1305::{ChaCha20Poly1305, Key, Nonce};
use hkdf::Hkdf;
use sha2::Sha512;

pub fn derive(salt: &str, info: &str, secret: &[u8]) -> [u8; 32] {
    let hk = Hkdf::<Sha512>::new(Some(salt.as_bytes()), secret);
    let mut out = [0u8; 32];
    hk.expand(info.as_bytes(), &mut out).unwrap();
    out
}

/// A 64-bit little-endian counter after four zero bytes.
pub fn nonce(counter: u64) -> [u8; 12] {
    let mut n = [0u8; 12];
    n[4..].copy_from_slice(&counter.to_le_bytes());
    n
}

pub struct Cipher {
    out: ChaCha20Poly1305,
    inp: ChaCha20Poly1305,
    out_counter: u64,
    in_counter: u64,
    pending: Vec<u8>,
}

impl Cipher {
    pub fn new(out_key: &[u8; 32], in_key: &[u8; 32]) -> Self {
        Cipher {
            out: ChaCha20Poly1305::new(Key::from_slice(out_key)),
            inp: ChaCha20Poly1305::new(Key::from_slice(in_key)),
            out_counter: 0,
            in_counter: 0,
            pending: Vec::new(),
        }
    }

    pub fn encrypt(&mut self, mut data: &[u8]) -> Vec<u8> {
        let mut out = Vec::new();
        while !data.is_empty() {
            let n = data.len().min(1024);
            let (frame, rest) = data.split_at(n);
            data = rest;
            let len = (n as u16).to_le_bytes();
            let nonce = nonce(self.out_counter);
            self.out_counter += 1;
            let ct = self
                .out
                .encrypt(Nonce::from_slice(&nonce), Payload { msg: frame, aad: &len })
                .unwrap();
            out.extend_from_slice(&len);
            out.extend_from_slice(&ct);
        }
        out
    }

    /// Takes bytes off the socket, returns whatever whole frames decrypt to.
    pub fn decrypt(&mut self, data: &[u8]) -> Result<Vec<u8>> {
        self.pending.extend_from_slice(data);
        let mut out = Vec::new();
        loop {
            if self.pending.len() < 2 {
                break;
            }
            let len = u16::from_le_bytes([self.pending[0], self.pending[1]]) as usize;
            if self.pending.len() < 2 + len + 16 {
                break;
            }
            let aad = [self.pending[0], self.pending[1]];
            let nonce = nonce(self.in_counter);
            self.in_counter += 1;
            let pt = self
                .inp
                .decrypt(
                    Nonce::from_slice(&nonce),
                    Payload { msg: &self.pending[2..2 + len + 16], aad: &aad },
                )
                .map_err(|_| anyhow!("couldn't decrypt a frame from the device"))?;
            out.extend_from_slice(&pt);
            self.pending.drain(..2 + len + 16);
        }
        Ok(out)
    }
}
