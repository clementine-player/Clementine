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

#ifndef INTERNET_PLEX_PLEXNOTIFICATIONLISTENER_H_
#define INTERNET_PLEX_PLEXNOTIFICATIONLISTENER_H_

#include <QByteArray>
#include <QObject>

#include "internet/plex/plexparser.h"

class QNetworkReply;
class QTimer;
class PlexService;

// Holds the server's event stream open and reports changes to music
// metadata.  Reconnects with backoff if the stream drops or goes quiet.
class PlexNotificationListener : public QObject {
  Q_OBJECT

 public:
  explicit PlexNotificationListener(PlexService* service,
                                    QObject* parent = nullptr);
  ~PlexNotificationListener() override;

  void Start();
  void Stop();

  // Interprets one event from the stream.  Public so it can be tested.
  // item_ids gets the rating keys of changed items; returns true if the event
  // says the library changed in a way that needs a sync.
  static bool IsLibraryChange(const PlexServerEvent& event,
                              QStringList* item_ids);

 signals:
  // item_ids may be empty if the change isn't tied to particular items, e.g.
  // a library scan finishing.
  void LibraryChanged(const QStringList& item_ids);

 private slots:
  void ReadyRead();
  void StreamEnded();
  void Reconnect();

 private:
  void ScheduleReconnect();

  PlexService* service_;
  QNetworkReply* reply_;
  PlexEventStreamParser parser_;
  QTimer* watchdog_;
  QTimer* reconnect_timer_;
  int backoff_msec_;
  bool running_;
};

#endif  // INTERNET_PLEX_PLEXNOTIFICATIONLISTENER_H_
