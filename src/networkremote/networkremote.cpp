/* This file is part of Clementine.
   Copyright 2012, Andreas Muttscheller <asfa194@gmail.com>

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

#include "core/logging.h"
#include "covers/currentartloader.h"
#include "playlist/playlistmanager.h"

#include "networkremote.h"

#include <QSettings>

const char* NetworkRemote::kSettingsGroup = "NetworkRemote";
const int NetworkRemote::kDefaultServerPort = 5500;

NetworkRemote::NetworkRemote(Application* app)
  : app_(app)
{
}


NetworkRemote::~NetworkRemote() {
  server_->close();
  delete incoming_xml_parser_;
  delete outgoing_xml_creator_;
}

void NetworkRemote::ReadSettings() {
  QSettings s;

  s.beginGroup(NetworkRemote::kSettingsGroup);
  use_remote_ = s.value("use_remote").toBool();
  port_       = s.value("port").toInt();
  if (port_ == 0) {
    port_ = kDefaultServerPort;
  }
  s.endGroup();
}

void NetworkRemote::SetupServer() {
  server_ = new QTcpServer();
  incoming_xml_parser_  = new IncomingXmlParser(app_);
  outgoing_xml_creator_ = new OutgoingXmlCreator(app_);

  connect(app_->current_art_loader(),
          SIGNAL(ArtLoaded(const Song&, const QString&, const QImage&)),
          outgoing_xml_creator_,
          SLOT(CurrentSongChanged(const Song&, const QString&, const QImage&)));
}

void NetworkRemote::StartServer() {
  if (!app_) {
    qLog(Error) << "Start Server called before having an application!";
    return;
  }
  // Check if user desires to start a network remote server
  ReadSettings();
  if (!use_remote_) {
    qLog(Info) << "Network Remote deactivated";
    return;
  }

  qLog(Info) << "Starting network remote";

  clients_ = NULL;

  connect(server_, SIGNAL(newConnection()), this, SLOT(AcceptConnection()));

  server_->listen(QHostAddress::Any, port_);

  qLog(Info) << "Listening on port " << port_;
}

void NetworkRemote::StopServer() {
  if (server_->isListening()) {
    server_->close();
  }
}

void NetworkRemote::ReloadSettings() {
  if (server_->isListening()) {
    server_->close();
  }
  StartServer();
}

void NetworkRemote::AcceptConnection() {
  if (!clients_) {
    // Create a new QList with clients
    clients_ = new QList<QTcpSocket*>();
    outgoing_xml_creator_->SetClients(clients_);

    // Setting up the signals, but only once
    connect(incoming_xml_parser_, SIGNAL(SendClementineInfos()),
            outgoing_xml_creator_, SLOT(SendClementineInfos()));
    connect(incoming_xml_parser_, SIGNAL(SendFirstData()),
            outgoing_xml_creator_, SLOT(SendFirstData()));
    connect(incoming_xml_parser_, SIGNAL(SendAllPlaylists()),
            outgoing_xml_creator_, SLOT(SendAllPlaylists()));
    connect(incoming_xml_parser_, SIGNAL(SendPlaylistSongs(int)),
            outgoing_xml_creator_, SLOT(SendPlaylistSongs(int)));

    connect(app_->playlist_manager(), SIGNAL(ActiveChanged(Playlist*)),
            outgoing_xml_creator_, SLOT(ActiveChanged(Playlist*)));
    connect(app_->playlist_manager(), SIGNAL(PlaylistChanged(Playlist*)),
            outgoing_xml_creator_, SLOT(ActiveChanged(Playlist*)));

    connect(app_->player(), SIGNAL(VolumeChanged(int)), outgoing_xml_creator_,
            SLOT(VolumeChanged(int)));
    connect(app_->player()->engine(), SIGNAL(StateChanged(Engine::State)),
            outgoing_xml_creator_, SLOT(StateChanged(Engine::State)));

    qLog(Info) << "Signals connected!";
  }
  QTcpSocket* client = server_->nextPendingConnection();

  clients_->push_back(client);

  // Connect to the slot IncomingData when receiving data
  connect(client, SIGNAL(readyRead()), this, SLOT(IncomingData()));
}

void NetworkRemote::IncomingData() {
  QByteArray buf;
  QTcpSocket* client =  static_cast<QTcpSocket*>(QObject::sender());

  buf = client->read(client->bytesAvailable());

  QString strbuf(buf);
  incoming_xml_parser_->Parse(&strbuf);

  qLog(Debug) << "Data = " << buf << "Size = " << buf.size();
  if (incoming_xml_parser_->close_connection()) {
    client->close();
  }
}
