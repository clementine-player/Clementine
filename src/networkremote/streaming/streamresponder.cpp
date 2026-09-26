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

#include "streamresponder.h"

#include <QTcpSocket>
#include <QTimer>

#include "core/logging.h"

const int StreamResponder::kIdleTimeoutMsec = 30000;

namespace {
const int kIdleCheckMsec = 5000;
}  // namespace

StreamResponder::StreamResponder(QTcpSocket* socket, const QByteArray& token,
                                 Kind kind, QObject* parent)
    : QObject(parent),
      socket_(socket),
      token_(token),
      kind_(kind),
      active_(true),
      idle_timer_(new QTimer(this)) {
  socket_->setParent(this);
  connect(socket_, SIGNAL(disconnected()), SLOT(Disconnected()));
  connect(socket_, SIGNAL(bytesWritten(qint64)), SLOT(Progress()));

  last_progress_.start();
  connect(idle_timer_, SIGNAL(timeout()), SLOT(CheckIdle()));
  idle_timer_->start(kIdleCheckMsec);
}

StreamResponder::~StreamResponder() {}

void StreamResponder::Disconnected() {
  active_ = false;
  deleteLater();
}

void StreamResponder::Progress() { last_progress_.restart(); }

void StreamResponder::CheckIdle() {
  // Waiting for more to send (radio, a slow upstream) is fine; a client that
  // won't take what's already queued isn't.
  if (socket_->bytesToWrite() > 0 &&
      last_progress_.elapsed() > kIdleTimeoutMsec) {
    qLog(Info) << "Dropping a media client that stopped reading:"
               << socket_->peerAddress().toString();
    socket_->abort();
  }
}
