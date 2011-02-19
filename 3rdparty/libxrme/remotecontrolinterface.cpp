/* This file is part of the XMPP Remote Media Extension.
   Copyright 2010, David Sansome <me@davidsansome.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "remotecontrolhandler.h"
#include "remotecontrolinterface.h"

namespace xrme {

struct RemoteControlInterface::Private {
  Private()
    : handler_(NULL) {}

  RemoteControlHandler* handler_;
};


RemoteControlInterface::RemoteControlInterface()
    : d(new Private) {
}

RemoteControlInterface::~RemoteControlInterface() {
}

void RemoteControlInterface::PlayPause(const QString& peer_jid_resource) {
  if (d->handler_) {
    d->handler_->PlayPause(peer_jid_resource);
  }
}

void RemoteControlInterface::Stop(const QString& peer_jid_resource) {
  if (d->handler_) {
    d->handler_->Stop(peer_jid_resource);
  }
}

void RemoteControlInterface::Next(const QString& peer_jid_resource) {
  if (d->handler_) {
    d->handler_->Next(peer_jid_resource);
  }
}

void RemoteControlInterface::Previous(const QString& peer_jid_resource) {
  if (d->handler_) {
    d->handler_->Previous(peer_jid_resource);
  }
}

void RemoteControlInterface::QueryState(const QString& peer_jid_resource) {
  if (d->handler_) {
    d->handler_->QueryState(peer_jid_resource);
  }
}

void RemoteControlInterface::Attach(RemoteControlHandler* handler) {
  d->handler_ = handler;
}

} // namespace xrme
