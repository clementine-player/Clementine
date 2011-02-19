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

#include "mediaplayerhandler.h"
#include "mediaplayerinterface.h"

#include <QtDebug>

namespace xrme {

struct MediaPlayerInterface::Private {
  Private()
    : handler_(NULL) {}

  MediaPlayerHandler* handler_;
};


MediaPlayerInterface::MediaPlayerInterface()
    : d(new Private) {
}

MediaPlayerInterface::~MediaPlayerInterface() {
}

void MediaPlayerInterface::StateChanged() {
  if (!d->handler_) {
    return;
  }

  d->handler_->StateChanged();
}

void MediaPlayerInterface::AlbumArtChanged() {
  if (!d->handler_) {
    return;
  }

  d->handler_->AlbumArtChanged();
}

void MediaPlayerInterface::Attach(MediaPlayerHandler* handler) {
  d->handler_ = handler;
}

} // namespace xrme
