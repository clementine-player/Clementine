/* This file is part of Clementine.
   Copyright 2026, John Maguire <john.maguire@gmail.com>

   Clementine is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Clementine is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Clementine.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef AIRPLAY_AIRPLAYCRYPTO_H_
#define AIRPLAY_AIRPLAYCRYPTO_H_

#include <QByteArray>
#include <concepts>

namespace airplay {

// The bytes of |value| as they are in memory, so convert it first:
// Bytes(qToBigEndian(seq)).
template <std::integral T>
QByteArray Bytes(T value) {
  return QByteArray(reinterpret_cast<const char*>(&value), sizeof(value));
}

QByteArray Sha512(const QByteArray& data);

// HKDF (RFC 5869) with HMAC-SHA512.
QByteArray HkdfSha512(const QByteArray& salt, const QByteArray& info,
                      const QByteArray& secret, int length = 32);

// The 12-byte ChaCha20-Poly1305 nonce HomeKit and AirPlay use: four zero
// bytes, then a 64-bit little-endian counter.
QByteArray Nonce(quint64 counter);

// ChaCha20-Poly1305 as in RFC 8439. Returns the ciphertext followed by the
// 16-byte tag.
QByteArray Seal(const QByteArray& key, const QByteArray& nonce,
                const QByteArray& aad, const QByteArray& plain);
// The reverse of Seal. False if the tag doesn't match.
bool Open(const QByteArray& key, const QByteArray& nonce, const QByteArray& aad,
          const QByteArray& sealed, QByteArray* plain);

// 32 random bytes, for keys.
QByteArray RandomKey();

// Ed25519 (RFC 8032), which HomeKit pairing signs with. A key pair is made
// from a 32-byte seed, which is what's kept.
QByteArray Ed25519PublicKey(const QByteArray& seed);
QByteArray Ed25519Sign(const QByteArray& seed, const QByteArray& message);
bool Ed25519Verify(const QByteArray& public_key, const QByteArray& signature,
                   const QByteArray& message);

// X25519 (RFC 7748), which pair-verify agrees on a shared secret with.
QByteArray X25519PublicKey(const QByteArray& secret);
QByteArray X25519Shared(const QByteArray& secret,
                        const QByteArray& their_public);

// The encryption AirPlay 2 puts on its TCP connections after pairing: frames
// of at most 1024 bytes, each preceded by its length (16-bit little endian),
// which is also the additional authenticated data.
class FrameCipher {
 public:
  FrameCipher(const QByteArray& out_key, const QByteArray& in_key);

  QByteArray Encrypt(const QByteArray& data);
  // Takes bytes off the socket, and appends whatever whole frames decrypt to
  // to |plain|. False if a frame doesn't decrypt.
  bool Decrypt(const QByteArray& data, QByteArray* plain);

 private:
  QByteArray out_key_;
  QByteArray in_key_;
  quint64 out_counter_ = 0;
  quint64 in_counter_ = 0;
  QByteArray pending_;
};

}  // namespace airplay

#endif  // AIRPLAY_AIRPLAYCRYPTO_H_
