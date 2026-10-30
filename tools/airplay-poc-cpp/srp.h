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

#ifndef AIRPLAY_SRP_H_
#define AIRPLAY_SRP_H_

#include <QByteArray>
#include <QString>

namespace airplay {

// The SRP-6a client of HomeKit and AirPlay 2 pairing: the 3072-bit group from
// RFC 5054, SHA-512 and the username "Pair-Setup". Integers are encoded as
// srptools (which receivers are checked against) does: minimal big-endian
// bytes, except where SRP pads them to the length of N.
class SrpClient {
 public:
  struct Result {
    // A, to send to the receiver.
    QByteArray public_key;
    // M1, to send to the receiver.
    QByteArray proof;
    // K = H(S), what the session keys are derived from.
    QByteArray key;
  };

  // With a random private value.
  SrpClient();
  // With the private value |a|, big endian, for tests.
  explicit SrpClient(const QByteArray& a);

  // Answers the receiver's |salt| and public key B with |pin|.
  Result Process(const QString& pin, const QByteArray& salt,
                 const QByteArray& b_public) const;

 private:
  QByteArray a_;
};

}  // namespace airplay

#endif  // AIRPLAY_SRP_H_
