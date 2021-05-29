/* This file is part of Clementine.
   Copyright 2021, Lukas Prediger <lumip@lumip.de>

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

#include "cddevice.h"

const int CdDevice::kDiscChangePollingIntervalMs = 500;

CdDevice::CdDevice(const QUrl& url, QObject* parent,
                   bool watch_for_disc_changes)
    : QObject(parent), url_(url), cdio_(nullptr), disc_changed_timer_() {
  cdio_ = cdio_open(url_.path().toLocal8Bit().constData(), DRIVER_DEVICE);
  connect(&disc_changed_timer_, SIGNAL(timeout()), SLOT(CheckDiscChanged()));
  WatchForDiscChanges(watch_for_disc_changes);
}

CdDevice::~CdDevice() {
  Q_ASSERT(cdio_);
  cdio_destroy(cdio_);
}

void CdDevice::WatchForDiscChanges(bool watch) {
  if (watch && !disc_changed_timer_.isActive())
    disc_changed_timer_.start(CdDevice::kDiscChangePollingIntervalMs);
  else if (!watch && disc_changed_timer_.isActive())
    disc_changed_timer_.stop();
}

void CdDevice::CheckDiscChanged() {
  Q_ASSERT(cdio_);
  if (cdio_get_media_changed(cdio_) == 1) emit DiscChanged();
}
