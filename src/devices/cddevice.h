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

#ifndef CDDEVICE_H
#define CDDEVICE_H

#include <QObject>
#include <QTimer>
#include <QUrl>

// Qt import must come first
#include <cdio/cdio.h>

class CdDevice : public QObject {
  Q_OBJECT

 public:
  Q_INVOKABLE CdDevice(const QUrl& url, QObject* parent = nullptr,
                       bool watch_for_disc_changes = true);
  ~CdDevice();

  const QUrl& url() const;
  void WatchForDiscChanges(bool watch);

  static const int kDiscChangePollingIntervalMs;

 signals:
  void DiscChanged();

 private slots:
  void CheckDiscChanged();

 private:
  QUrl url_;
  CdIo_t* cdio_;
  QTimer disc_changed_timer_;
};

#endif
