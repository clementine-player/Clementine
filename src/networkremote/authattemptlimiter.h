/* This file is part of Clementine.

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

#ifndef NETWORKREMOTE_AUTHATTEMPTLIMITER_H
#define NETWORKREMOTE_AUTHATTEMPTLIMITER_H

#include <QElapsedTimer>
#include <QHash>
#include <QString>
#include <functional>

class QHostAddress;

// Limits how fast one address can guess the network remote's auth code.
//
// The code is a number below 100000, so without a limit a script can try
// them all in minutes. A few wrong codes in a row are free, for someone who
// mistypes it. After that the address's codes aren't checked for a while,
// starting at 10 seconds and doubling with each further wrong code, up to an
// hour. Once there, it gets about 24 guesses a day: on average it would take
// over five years to find a code. Someone who mistyped it six times waits ten
// seconds.
//
// A right code forgets the address, as does a day without a wrong code from
// it. That has to be longer than the longest wait, or waiting it out would
// buy more free guesses.
//
// IPv6 addresses are counted by their /64: one host usually has a whole /64
// to pick addresses from.
class AuthAttemptLimiter {
 public:
  // Milliseconds on a clock that only goes forwards.
  typedef std::function<qint64()> Clock;

  static const int kFreeFailures;
  static const qint64 kFirstLockoutMsec;
  static const qint64 kMaxLockoutMsec;
  static const qint64 kForgetAfterMsec;
  // Beyond this many addresses, the ones with the oldest wrong codes are
  // forgotten first.
  static const int kMaxAddresses;

  // Uses |clock| for the time if it's given; tests give one.
  explicit AuthAttemptLimiter(Clock clock = Clock());

  // How long until a code from |address| is checked again, in milliseconds:
  // 0 if it should be checked now. Logs refusals.
  qint64 LockedOutFor(const QHostAddress& address);

  // A wrong code from |address|. Returns how long it's now refused for, in
  // milliseconds: 0 while it's within its free failures.
  qint64 RecordFailure(const QHostAddress& address);

  // A right code from |address|.
  void RecordSuccess(const QHostAddress& address);

  int address_count() const { return records_.size(); }

  // Addresses that share a key share a count.
  static QString KeyFor(const QHostAddress& address);

 private:
  struct Record {
    int failures = 0;
    qint64 last_failure = 0;
    qint64 locked_until = 0;
    // So a lockout's refusals are logged once rather than every time.
    bool refusal_logged = false;
  };

  qint64 Now() const;
  // The record for |key|, if there is one that hasn't been forgotten.
  Record* Find(const QString& key, qint64 now);
  void MakeRoom(qint64 now);

  Clock clock_;
  QElapsedTimer timer_;
  QHash<QString, Record> records_;
};

#endif  // NETWORKREMOTE_AUTHATTEMPTLIMITER_H
