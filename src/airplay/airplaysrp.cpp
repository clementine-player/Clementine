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

#include "airplaysrp.h"

#include <QRandomGenerator>

#include "airplaycrypto.h"
#include "mini-gmp.h"

namespace airplay {

namespace {

// The 3072-bit group of RFC 5054 Appendix A (the same prime as RFC 3526's
// group 15), with generator 5, which HomeKit pairing uses.
const char kN[] =
    "FFFFFFFFFFFFFFFFC90FDAA22168C234C4C6628B80DC1CD129024E088A67CC74020BBEA6"
    "3B139B22514A08798E3404DDEF9519B3CD3A431B302B0A6DF25F14374FE1356D6D51C245"
    "E485B576625E7EC6F44C42E9A637ED6B0BFF5CB6F406B7EDEE386BFB5A899FA5AE9F2411"
    "7C4B1FE649286651ECE45B3DC2007CB8A163BF0598DA48361C55D39A69163FA8FD24CF5F"
    "83655D23DCA3AD961C62F356208552BB9ED529077096966D670C354E4ABC9804F1746C08"
    "CA18217C32905E462E36CE3BE39E772C180E86039B2783A2EC07A28FB5C55DF06F4C52C9"
    "DE2BCBF6955817183995497CEA956AE515D2261898FA051015728E5A8AAAC42DAD33170D"
    "04507A33A85521ABDF1CBA64ECFB850458DBEF0A8AEA71575D060C7DB3970F85A6E1E4C7"
    "ABF5AE8CDB0933D71E8C94E04A25619DCEE3D2261AD2EE6BF12FFA06D98A0864D8760273"
    "3EC86A64521F2B18177B200CBBE117577A615D6C770988C0BAD946E208E24FA074E5AB31"
    "43DB5BFCE0FD108E4B82D120A93AD2CAFFFFFFFFFFFFFFFF";
const int kG = 5;
// The SRP username, I, which HomeKit pairing fixes.
const char kUsername[] = "Pair-Setup";

// An mpz_t that frees itself.
class BigInt {
 public:
  BigInt() { mpz_init(v_); }
  explicit BigInt(const QByteArray& big_endian) {
    mpz_init(v_);
    mpz_import(v_, big_endian.size(), 1, 1, 1, 0, big_endian.constData());
  }
  ~BigInt() { mpz_clear(v_); }
  BigInt(const BigInt&) = delete;
  BigInt& operator=(const BigInt&) = delete;

  mpz_ptr get() { return v_; }
  mpz_srcptr get() const { return v_; }

  // Minimal big-endian bytes: no leading zeros, but one zero byte for 0.
  QByteArray Bytes() const {
    const size_t size = (mpz_sizeinbase(v_, 2) + 7) / 8;
    QByteArray out(size, '\0');
    size_t written = 0;
    mpz_export(out.data(), &written, 1, 1, 1, 0, v_);
    return out.left(qMax<size_t>(written, 1));
  }

  // Big-endian bytes, padded with leading zeros to |size|.
  QByteArray Padded(int size) const {
    const QByteArray b = Bytes();
    return QByteArray(qMax(0, size - int(b.size())), '\0') + b;
  }

 private:
  mpz_t v_;
};

}  // namespace

SrpClient::SrpClient() {
  a_.resize(32);
  QRandomGenerator::system()->fillRange(reinterpret_cast<quint32*>(a_.data()),
                                        a_.size() / sizeof(quint32));
}

SrpClient::SrpClient(const QByteArray& a) : a_(a) {}

SrpClient::Result SrpClient::Process(const QString& pin,
                                     const QByteArray& salt_bytes,
                                     const QByteArray& b_public) const {
  BigInt n;
  mpz_set_str(n.get(), kN, 16);
  BigInt g;
  mpz_set_ui(g.get(), kG);
  const int n_size = n.Bytes().size();

  BigInt a(a_);
  BigInt a_pub;
  mpz_powm(a_pub.get(), g.get(), a.get(), n.get());

  BigInt b(b_public);
  // srptools hashes the salt as an integer, so drops its leading zeros.
  const QByteArray salt = BigInt(salt_bytes).Bytes();

  const BigInt k(Sha512(n.Bytes() + g.Padded(n_size)));
  const BigInt u(Sha512(a_pub.Padded(n_size) + b.Padded(n_size)));
  const QByteArray inner = Sha512(QByteArray(kUsername) + ":" + pin.toUtf8());
  const BigInt x(Sha512(salt + inner));
  BigInt v;
  mpz_powm(v.get(), g.get(), x.get(), n.get());

  // S = (B - k * v) ^ (a + u * x) mod N
  BigInt kv;
  mpz_mul(kv.get(), k.get(), v.get());
  mpz_mod(kv.get(), kv.get(), n.get());
  BigInt base;
  mpz_mod(base.get(), b.get(), n.get());
  mpz_add(base.get(), base.get(), n.get());
  mpz_sub(base.get(), base.get(), kv.get());
  mpz_mod(base.get(), base.get(), n.get());
  BigInt exponent;
  mpz_mul(exponent.get(), u.get(), x.get());
  mpz_add(exponent.get(), exponent.get(), a.get());
  BigInt s;
  mpz_powm(s.get(), base.get(), exponent.get(), n.get());

  Result result;
  result.key = Sha512(s.Bytes());

  // M1 = H(H(N) xor H(g), H(I), s, A, B, K)
  const BigInt hn(Sha512(n.Bytes()));
  const BigInt hg(Sha512(g.Bytes()));
  BigInt hxor;
  mpz_xor(hxor.get(), hn.get(), hg.get());
  result.proof = Sha512(hxor.Bytes() + Sha512(kUsername) + salt +
                        a_pub.Bytes() + b.Bytes() + result.key);
  result.public_key = a_pub.Bytes();
  return result;
}

}  // namespace airplay
