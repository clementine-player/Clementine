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

#include "airplaycrypto.h"

#include <QCryptographicHash>
#include <QMessageAuthenticationCode>
#include <QRandomGenerator>
#include <QtEndian>

#include "monocypher-ed25519.h"
#include "monocypher.h"

namespace airplay {

namespace {

// Poly1305's tag.
const int kTagSize = 16;
// The most plaintext in one frame of an encrypted HomeKit connection, from
// the HomeKit Accessory Protocol specification.
const int kMaxFrame = 1024;

const uint8_t* Bytes(const QByteArray& data) {
  return reinterpret_cast<const uint8_t*>(data.constData());
}

uint8_t* Bytes(QByteArray* data) {
  return reinterpret_cast<uint8_t*>(data->data());
}

}  // namespace

QByteArray Sha512(const QByteArray& data) {
  return QCryptographicHash::hash(data, QCryptographicHash::Sha512);
}

QByteArray HkdfSha512(const QByteArray& salt, const QByteArray& info,
                      const QByteArray& secret, int length) {
  const QByteArray prk = QMessageAuthenticationCode::hash(
      secret, salt, QCryptographicHash::Sha512);
  QByteArray okm;
  QByteArray block;
  for (char i = 1; okm.size() < length; ++i) {
    block = QMessageAuthenticationCode::hash(QByteArray(block + info + char(i)),
                                             prk, QCryptographicHash::Sha512);
    okm += block;
  }
  return okm.left(length);
}

QByteArray Nonce(quint64 counter) {
  return QByteArray(4, '\0') + Bytes(qToLittleEndian(counter));
}

QByteArray Seal(const QByteArray& key, const QByteArray& nonce,
                const QByteArray& aad, const QByteArray& plain) {
  Q_ASSERT(key.size() == 32 && nonce.size() == 12);
  // Monocypher's AEAD context changes its key after each message, so each
  // message gets a new one: the first message is plain RFC 8439.
  crypto_aead_ctx ctx;
  crypto_aead_init_ietf(&ctx, Bytes(key), Bytes(nonce));
  QByteArray sealed(plain.size() + kTagSize, '\0');
  crypto_aead_write(&ctx, Bytes(&sealed), Bytes(&sealed) + plain.size(),
                    Bytes(aad), aad.size(), Bytes(plain), plain.size());
  crypto_wipe(&ctx, sizeof(ctx));
  return sealed;
}

bool Open(const QByteArray& key, const QByteArray& nonce, const QByteArray& aad,
          const QByteArray& sealed, QByteArray* plain) {
  Q_ASSERT(key.size() == 32 && nonce.size() == 12);
  if (sealed.size() < kTagSize) return false;
  const int size = sealed.size() - kTagSize;
  crypto_aead_ctx ctx;
  crypto_aead_init_ietf(&ctx, Bytes(key), Bytes(nonce));
  QByteArray out(size, '\0');
  const int mismatch =
      crypto_aead_read(&ctx, Bytes(&out), Bytes(sealed) + size, Bytes(aad),
                       aad.size(), Bytes(sealed), size);
  crypto_wipe(&ctx, sizeof(ctx));
  if (mismatch) return false;
  *plain = out;
  return true;
}

QByteArray RandomKey() {
  QByteArray key(32, '\0');
  QRandomGenerator::system()->fillRange(reinterpret_cast<quint32*>(key.data()),
                                        key.size() / sizeof(quint32));
  return key;
}

namespace {

// Monocypher's Ed25519 secret key: the seed, then the public key.
QByteArray Ed25519SecretKey(const QByteArray& seed, QByteArray* public_key) {
  Q_ASSERT(seed.size() == 32);
  QByteArray seed_copy = seed;  // Wiped by Monocypher
  QByteArray secret(64, '\0');
  public_key->resize(32);
  crypto_ed25519_key_pair(Bytes(&secret), Bytes(public_key), Bytes(&seed_copy));
  return secret;
}

}  // namespace

QByteArray Ed25519PublicKey(const QByteArray& seed) {
  QByteArray public_key;
  QByteArray secret = Ed25519SecretKey(seed, &public_key);
  crypto_wipe(Bytes(&secret), secret.size());
  return public_key;
}

QByteArray Ed25519Sign(const QByteArray& seed, const QByteArray& message) {
  QByteArray public_key;
  QByteArray secret = Ed25519SecretKey(seed, &public_key);
  QByteArray signature(64, '\0');
  crypto_ed25519_sign(Bytes(&signature), Bytes(secret), Bytes(message),
                      message.size());
  crypto_wipe(Bytes(&secret), secret.size());
  return signature;
}

bool Ed25519Verify(const QByteArray& public_key, const QByteArray& signature,
                   const QByteArray& message) {
  if (public_key.size() != 32 || signature.size() != 64) return false;
  return crypto_ed25519_check(Bytes(signature), Bytes(public_key),
                              Bytes(message), message.size()) == 0;
}

QByteArray X25519PublicKey(const QByteArray& secret) {
  Q_ASSERT(secret.size() == 32);
  QByteArray public_key(32, '\0');
  crypto_x25519_public_key(Bytes(&public_key), Bytes(secret));
  return public_key;
}

QByteArray X25519Shared(const QByteArray& secret,
                        const QByteArray& their_public) {
  if (their_public.size() != 32) return QByteArray();
  QByteArray shared(32, '\0');
  crypto_x25519(Bytes(&shared), Bytes(secret), Bytes(their_public));
  return shared;
}

FrameCipher::FrameCipher(const QByteArray& out_key, const QByteArray& in_key)
    : out_key_(out_key), in_key_(in_key) {}

QByteArray FrameCipher::Encrypt(const QByteArray& data) {
  QByteArray out;
  for (int pos = 0; pos < data.size(); pos += kMaxFrame) {
    const QByteArray frame = data.mid(pos, kMaxFrame);
    const QByteArray length = Bytes(qToLittleEndian<quint16>(frame.size()));
    out += length;
    out += Seal(out_key_, Nonce(out_counter_++), length, frame);
  }
  return out;
}

bool FrameCipher::Decrypt(const QByteArray& data, QByteArray* plain) {
  pending_ += data;
  while (pending_.size() >= 2) {
    const int length = qFromLittleEndian<quint16>(pending_.constData());
    if (pending_.size() < 2 + length + kTagSize) break;
    QByteArray frame;
    if (!Open(in_key_, Nonce(in_counter_++), pending_.left(2),
              pending_.mid(2, length + kTagSize), &frame)) {
      return false;
    }
    *plain += frame;
    pending_.remove(0, 2 + length + kTagSize);
  }
  return true;
}

}  // namespace airplay
