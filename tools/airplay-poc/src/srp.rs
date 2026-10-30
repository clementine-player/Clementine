//! SRP-6a client as used by HomeKit / AirPlay 2 pairing: 3072-bit group,
//! SHA-512, username "Pair-Setup", and the integer encodings of srptools
//! (minimal big-endian bytes, except where the SRP spec pads to N's length).

use num_bigint::BigUint;
use rand::RngCore;
use sha2::{Digest, Sha512};

const N_HEX: &str = "FFFFFFFFFFFFFFFFC90FDAA22168C234C4C6628B80DC1CD129024E088A67CC74020BBEA63B139B22514A08798E3404DDEF9519B3CD3A431B302B0A6DF25F14374FE1356D6D51C245E485B576625E7EC6F44C42E9A637ED6B0BFF5CB6F406B7EDEE386BFB5A899FA5AE9F24117C4B1FE649286651ECE45B3DC2007CB8A163BF0598DA48361C55D39A69163FA8FD24CF5F83655D23DCA3AD961C62F356208552BB9ED529077096966D670C354E4ABC9804F1746C08CA18217C32905E462E36CE3BE39E772C180E86039B2783A2EC07A28FB5C55DF06F4C52C9DE2BCBF6955817183995497CEA956AE515D2261898FA051015728E5A8AAAC42DAD33170D04507A33A85521ABDF1CBA64ECFB850458DBEF0A8AEA71575D060C7DB3970F85A6E1E4C7ABF5AE8CDB0933D71E8C94E04A25619DCEE3D2261AD2EE6BF12FFA06D98A0864D87602733EC86A64521F2B18177B200CBBE117577A615D6C770988C0BAD946E208E24FA074E5AB3143DB5BFCE0FD108E4B82D120A93AD2CAFFFFFFFFFFFFFFFF";
const USERNAME: &str = "Pair-Setup";

fn hash(parts: &[&[u8]]) -> Vec<u8> {
    let mut h = Sha512::new();
    for p in parts {
        h.update(p);
    }
    h.finalize().to_vec()
}

fn int(bytes: &[u8]) -> BigUint {
    BigUint::from_bytes_be(bytes)
}

pub struct Client {
    n: BigUint,
    g: BigUint,
    a: BigUint,
    a_pub: BigUint,
}

pub struct Session {
    /// A, to send to the device.
    pub public: Vec<u8>,
    /// M1, to send to the device.
    pub proof: Vec<u8>,
    /// K = H(S), what the encryption keys are derived from.
    pub key: Vec<u8>,
}

impl Client {
    pub fn new() -> Self {
        let mut a_bytes = [0u8; 32];
        rand::thread_rng().fill_bytes(&mut a_bytes);
        Self::with_private(&a_bytes)
    }

    pub fn with_private(a_bytes: &[u8]) -> Self {
        let n = BigUint::parse_bytes(N_HEX.as_bytes(), 16).unwrap();
        let g = BigUint::from(5u32);
        let a = int(a_bytes);
        let a_pub = g.modpow(&a, &n);
        Client { n, g, a, a_pub }
    }

    fn pad(&self, v: &BigUint) -> Vec<u8> {
        let len = self.n.to_bytes_be().len();
        let b = v.to_bytes_be();
        let mut out = vec![0u8; len - b.len()];
        out.extend_from_slice(&b);
        out
    }

    /// Answers the device's salt and public key B with the PIN.
    pub fn process(&self, pin: &str, salt: &[u8], b_pub: &[u8]) -> Session {
        let n = &self.n;
        let b = int(b_pub);
        // srptools hashes the salt as an integer, so drops leading zeros.
        let salt = int(salt).to_bytes_be();

        let k = int(&hash(&[&n.to_bytes_be(), &self.pad(&self.g)]));
        let u = int(&hash(&[&self.pad(&self.a_pub), &self.pad(&b)]));
        let inner = hash(&[format!("{USERNAME}:{pin}").as_bytes()]);
        let x = int(&hash(&[&salt, &inner]));
        let v = self.g.modpow(&x, n);

        let kv = (&k * &v) % n;
        let base = ((&b % n) + n - kv) % n;
        let s = base.modpow(&(&self.a + &u * &x), n);
        let key = hash(&[&s.to_bytes_be()]);

        let hn = int(&hash(&[&n.to_bytes_be()]));
        let hg = int(&hash(&[&self.g.to_bytes_be()]));
        let hxor = (hn ^ hg).to_bytes_be();
        let hi = hash(&[USERNAME.as_bytes()]);
        let proof = hash(&[
            &hxor,
            &hi,
            &salt,
            &self.a_pub.to_bytes_be(),
            &b.to_bytes_be(),
            &key,
        ]);

        Session { public: self.a_pub.to_bytes_be(), proof, key }
    }
}
