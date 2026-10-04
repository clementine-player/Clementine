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

#include "networkremote/authattemptlimiter.h"

#include <QHostAddress>
#include <algorithm>

#include "core/logging.h"

const int AuthAttemptLimiter::kFreeFailures = 5;
const qint64 AuthAttemptLimiter::kFirstLockoutMsec = 10 * 1000;
const qint64 AuthAttemptLimiter::kMaxLockoutMsec = 60 * 60 * 1000;
const qint64 AuthAttemptLimiter::kForgetAfterMsec = 24 * 60 * 60 * 1000;
const int AuthAttemptLimiter::kMaxAddresses = 4096;

AuthAttemptLimiter::AuthAttemptLimiter(Clock clock) : clock_(clock) {
  timer_.start();
}

qint64 AuthAttemptLimiter::Now() const {
  return clock_ ? clock_() : timer_.elapsed();
}

QString AuthAttemptLimiter::KeyFor(const QHostAddress& address) {
  // IPv4 clients of a dual-stack socket arrive as ::ffff:a.b.c.d.
  bool is_ipv4 = false;
  const quint32 ipv4 = address.toIPv4Address(&is_ipv4);
  if (is_ipv4) return QHostAddress(ipv4).toString();

  Q_IPV6ADDR ipv6 = address.toIPv6Address();
  std::fill(ipv6.c + 8, ipv6.c + 16, 0);
  return QHostAddress(ipv6).toString() + "/64";
}

AuthAttemptLimiter::Record* AuthAttemptLimiter::Find(const QString& key,
                                                     qint64 now) {
  auto it = records_.find(key);
  if (it == records_.end()) return nullptr;
  if (now - it->last_failure >= kForgetAfterMsec) {
    records_.erase(it);
    return nullptr;
  }
  return &it.value();
}

bool AuthAttemptLimiter::MayTry(const QHostAddress& address) {
  const qint64 now = Now();
  Record* record = Find(KeyFor(address), now);
  if (!record || now >= record->locked_until) return true;

  const qint64 seconds_left = (record->locked_until - now + 999) / 1000;
  if (record->refusal_logged) {
    qLog(Debug) << "Refusing an auth code from" << address.toString()
                << "for another" << seconds_left << "seconds";
  } else {
    record->refusal_logged = true;
    qLog(Warning) << "Refusing an auth code from" << address.toString()
                  << "for another" << seconds_left << "seconds, after"
                  << record->failures << "wrong codes";
  }
  return false;
}

qint64 AuthAttemptLimiter::RecordFailure(const QHostAddress& address) {
  const qint64 now = Now();
  const QString key = KeyFor(address);
  Record* record = Find(key, now);
  if (!record) {
    MakeRoom(now);
    record = &records_[key];
  }

  record->failures++;
  record->last_failure = now;

  qint64 lockout = 0;
  const int over = record->failures - kFreeFailures;
  if (over > 0) {
    lockout = kFirstLockoutMsec;
    for (int i = 1; i < over && lockout < kMaxLockoutMsec; ++i) lockout *= 2;
    lockout = std::min(lockout, kMaxLockoutMsec);
    record->locked_until = now + lockout;
    record->refusal_logged = false;
    qLog(Warning) << "Wrong auth code from" << address.toString() << "("
                  << record->failures << "in a row); refusing its codes for"
                  << lockout / 1000 << "seconds";
  } else {
    qLog(Warning) << "Wrong auth code from" << address.toString() << "("
                  << record->failures << "in a row)";
  }
  return lockout;
}

void AuthAttemptLimiter::RecordSuccess(const QHostAddress& address) {
  records_.remove(KeyFor(address));
}

void AuthAttemptLimiter::MakeRoom(qint64 now) {
  if (records_.size() < kMaxAddresses) return;

  for (auto it = records_.begin(); it != records_.end();) {
    if (now - it->last_failure >= kForgetAfterMsec) {
      it = records_.erase(it);
    } else {
      ++it;
    }
  }
  if (records_.size() < kMaxAddresses) return;

  // Forgetting someone frees up their guesses, but only one at a time, and
  // whoever has this many addresses has those guesses anyway.
  auto oldest = records_.begin();
  for (auto it = records_.begin(); it != records_.end(); ++it) {
    if (it->last_failure < oldest->last_failure) oldest = it;
  }
  qLog(Debug) << "Forgetting wrong auth codes from" << oldest.key()
              << "to make room";
  records_.erase(oldest);
}
