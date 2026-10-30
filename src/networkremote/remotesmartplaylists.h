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

#ifndef NETWORKREMOTE_REMOTESMARTPLAYLISTS_H_
#define NETWORKREMOTE_REMOTESMARTPLAYLISTS_H_

#include <QByteArray>
#include <QList>
#include <QObject>
#include <QString>
#include <functional>

#include "remotecontrolmessages.pb.h"
#include "smartplaylists/generator_fwd.h"

class QMimeData;

// Lists the library's smart playlists to network remote clients, and puts one
// on the playlist as double-clicking it in the library does.
//
// Lives on the main thread with the library model. The network remote's
// thread talks to it through queued signals carrying serialized messages, as
// with InternetBrowser.
class RemoteSmartPlaylists : public QObject {
  Q_OBJECT

 public:
  struct Hooks {
    // The smart playlists, in the order the library shows them.
    std::function<QList<cpb::remote::SmartPlaylist>()> list;
    // The generator for the smart playlist with this id, or null if there's
    // none.
    std::function<smart_playlists::GeneratorPtr(const QString& id)> generator;
  };

  explicit RemoteSmartPlaylists(const Hooks& hooks, QObject* parent = nullptr);

 public slots:
  void HandleMessage(int client_id, const QByteArray& data);

 signals:
  void SendToClient(int client_id, const QByteArray& data);
  // As the library's double-click does; the receiver takes ownership.
  void AddToPlaylist(QMimeData* data);

 private:
  void Add(const cpb::remote::RequestSmartPlaylistAdd& request,
           cpb::remote::ResponseSmartPlaylistAdd* response);

  Hooks hooks_;
};

#endif  // NETWORKREMOTE_REMOTESMARTPLAYLISTS_H_
