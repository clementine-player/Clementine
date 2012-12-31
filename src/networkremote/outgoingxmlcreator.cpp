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

#include "outgoingxmlcreator.h"
#include "remotexmltags.h"
#include "core/logging.h"

OutgoingXmlCreator::OutgoingXmlCreator(Application* app)
  : app_(app),
    clients_(NULL)
{
  // Create Keep Alive Timer
  keep_alive_timer_ = new QTimer(this);
  connect(keep_alive_timer_, SIGNAL(timeout()), this, SLOT(SendKeepAlive()));
  keep_alive_timeout_ = 10000;
}

OutgoingXmlCreator::~OutgoingXmlCreator() {
}

void OutgoingXmlCreator::SetClients(QList<QTcpSocket*>* clients) {
  clients_ = clients;
  // After we got some clients, start the keep alive timer
  // Default: every 10 seconds
  keep_alive_timer_->start(keep_alive_timeout_);
}

void OutgoingXmlCreator::SendDataToClients(QByteArray data) {
  // Check if we have clients to send data to
  if (!clients_) {
    return;
  }

  QTcpSocket* sock;
  foreach(sock, *clients_) {
    // Check if the client is still active
    if (sock->state() == QTcpSocket::ConnectedState) {
      sock->write(data);
    } else {
      clients_->removeAt(clients_->indexOf(sock));
    }
  }
}

void OutgoingXmlCreator::SendClementineInfos() {
  // Create the xml and header
  QDomDocument doc;
  CreateXmlHeader(&doc, RemoteXmlTags::SEND_INFOS);
  QDomElement root = doc.documentElement();
  QDomElement info = doc.createElement(RemoteXmlTags::INFOS);
  QDomElement tag_version = doc.createElement(RemoteXmlTags::VERSION);
  QDomText version_text = doc.createTextNode(QString("%1 %2").arg(
                                                QCoreApplication::applicationName(),
                                                QCoreApplication::applicationVersion()));

  tag_version.appendChild(version_text);
  info.appendChild(tag_version);
  root.appendChild(info);

  SendDataToClients(doc.toByteArray());
}

void OutgoingXmlCreator::SendAllPlaylists() {
  // Get all Playlists
  QList<Playlist*> playlists = app_->playlist_manager()->GetAllPlaylists();
  QListIterator<Playlist*> i(playlists);
  int active_playlist = app_->playlist_manager()->active_id();

  // Create the xml and header
  QDomDocument doc;
  CreateXmlHeader(&doc, RemoteXmlTags::SEND_PLAYLISTS);
  QDomElement root = doc.documentElement();

  while(i.hasNext()) {
    Playlist* p = i.next();

    // Append the id
    QDomElement node_playlist = doc.createElement(RemoteXmlTags::PLAYLIST);
    node_playlist.setAttribute(RemoteXmlTags::ID, p->id());
    node_playlist.setAttribute(RemoteXmlTags::CURRENT_ROW, p->current_row());

    // get the name of the playlist
    QDomElement playlist_elem = doc.createElement(RemoteXmlTags::PLAYLIST_NAME);
    QString playlist_name = app_->playlist_manager()->GetPlaylistName(p->id());
    QDomText    playlist_text = doc.createTextNode(playlist_name);

    // Add it to the playlist node
    playlist_elem.appendChild(playlist_text);
    node_playlist.appendChild(playlist_elem);

    // get the item count
    QDomElement playlist_count = doc.createElement(RemoteXmlTags::PLAYLIST_ITEMS);
    QDomText item_count = doc.createTextNode(QString::number(p->GetAllSongs().size()));
    playlist_count.appendChild(item_count);
    node_playlist.appendChild(playlist_count);

    // Check if the Playlist is active
    QDomElement playlist_active_elem = doc.createElement(RemoteXmlTags::PLAYLIST_ACTIVE);
    QDomText text_active = doc.createTextNode( QString::number( (p->id() == active_playlist) ) );
    playlist_active_elem.appendChild(text_active);
    node_playlist.appendChild(playlist_active_elem);

    // Append it to the root node
    root.appendChild(node_playlist);
  }

  SendDataToClients(doc.toByteArray());
}

void OutgoingXmlCreator::ActiveChanged(Playlist *) {
  // When a playlist was changed, send the new list
  SendAllPlaylists();
}

void OutgoingXmlCreator::SendFirstData() {
  // First Send the current song
  PlaylistItemPtr item = app_->player()->GetCurrentItem();
  if (!item) {
    qLog(Info) << "No current item found!";
  }

  CurrentSongChanged(current_song_, current_uri_, current_image_);

  // then the current volume
  VolumeChanged(app_->player()->GetVolume());
}

void OutgoingXmlCreator::CurrentSongChanged(const Song& song, const QString& uri, const QImage& img) {
  current_song_  = song;
  current_uri_   = uri;
  current_image_ = img;

  if (clients_) {
    // Create the xml and header
    QDomDocument doc;
    CreateXmlHeader(&doc, RemoteXmlTags::SEND_METAINFOS);
    QDomElement root = doc.documentElement();

    // If there is no song, create an empty node, otherwise fill it with data
    int i = app_->playlist_manager()->active()->current_row();
    root.appendChild(CreateSong(&doc, &current_song_, &uri, i));

    SendDataToClients(doc.toByteArray());
  }
}

QDomElement OutgoingXmlCreator::CreateSong(QDomDocument* doc, Song* song, const QString* artUri, int index) {
  QDomElement nodeSong = doc->createElement(RemoteXmlTags::SONG);
  if (song->is_valid()) {
    QString pretty_length = song->PrettyLength();
    QString pretty_year   = song->PrettyYear();
    nodeSong.setAttribute(RemoteXmlTags::ID, song->id());
    nodeSong.setAttribute(RemoteXmlTags::INDEX, index);
    nodeSong.appendChild(CreateSongTag(doc, RemoteXmlTags::SONG_TITLE,       song->PrettyTitle() ));
    nodeSong.appendChild(CreateSongTag(doc, RemoteXmlTags::SONG_ARTIST,      song->artist() ));
    nodeSong.appendChild(CreateSongTag(doc, RemoteXmlTags::SONG_ALBUM,       song->album() ));
    nodeSong.appendChild(CreateSongTag(doc, RemoteXmlTags::SONG_ALBUMARTIST, song->albumartist() ));
    nodeSong.appendChild(CreateSongTag(doc, RemoteXmlTags::SONG_LENGTH,      pretty_length ));
    nodeSong.appendChild(CreateSongTag(doc, RemoteXmlTags::SONG_GENRE,       song->genre() ));
    nodeSong.appendChild(CreateSongTag(doc, RemoteXmlTags::SONG_YEAR,        pretty_year ));
    nodeSong.appendChild(CreateSongTag(doc, RemoteXmlTags::SONG_TRACK,       QString::number(song->track()) ));
    nodeSong.appendChild(CreateSongTag(doc, RemoteXmlTags::SONG_DISC,        QString::number(song->disc()) ));
    nodeSong.appendChild(CreateSongTag(doc, RemoteXmlTags::SONG_PLAYCOUNT,   QString::number(song->playcount()) ));
    // Append coverart
    if (!artUri->isEmpty()) {
      QImage orig(QUrl(*artUri).toLocalFile());
      QImage small;
      // Check if we resize the image
      if (orig.width() > 1000) {
        small = orig.scaled(1000, 1000, Qt::KeepAspectRatio);
      } else {
        small = orig;
      }

      // Read the image in a buffer and compress it
      QByteArray data;
      QBuffer buf(&data);
      buf.open(QIODevice::WriteOnly);
      small.save(&buf, "JPG");

      // Append the Data in the xml file
      QDomElement art = doc->createElement(RemoteXmlTags::SONG_ART);
      // Art must be sent in Base64 encoding, because the raw data escapes QString and
      // not all data is transfered
      QDomText art_content = doc->createTextNode(data.toBase64());
      art.appendChild(art_content);
      nodeSong.appendChild(art);
      buf.close();
    }
  }

  return nodeSong;
}

QDomElement OutgoingXmlCreator::CreateSongTag(QDomDocument* doc, QString tag, QString text) {
  QDomElement elem   = doc->createElement(tag);
  QDomText elem_text = doc->createTextNode(text);
  elem.appendChild(elem_text);
  return elem;
}

void OutgoingXmlCreator::VolumeChanged(int volume) {
  // Create the xml and header
  QDomDocument doc;
  CreateXmlHeader(&doc, RemoteXmlTags::VOLUME);
  QDomElement root = doc.documentElement();

  QDomElement volume_elem = doc.createElement(RemoteXmlTags::VOLUME);
  QDomText    volume_text = doc.createTextNode(QString::number(volume));
  volume_elem.appendChild(volume_text);
  root.appendChild(volume_elem);

  SendDataToClients(doc.toByteArray());
}

void OutgoingXmlCreator::SendPlaylistSongs(int id) {
  // Get the Playlist
  Playlist* playlist = app_->playlist_manager()->playlist(id);
  if(!playlist) {
    qLog(Info) << "Could not find playlist with id = " << id;
    return;
  }

  SongList song_list = playlist->GetAllSongs();
  QListIterator<Song> i(song_list);

  // Create the xml and header
  QDomDocument doc;
  CreateXmlHeader(&doc, RemoteXmlTags::SEND_PLAYLIST_SONGS);
  QDomElement root = doc.documentElement();

  QDomElement playlist_elem = doc.createElement(RemoteXmlTags::PLAYLIST);
  playlist_elem.setAttribute(RemoteXmlTags::ID, id);

  // Send all songs
  int index = 0;
  while(i.hasNext()) {
    Song song   = i.next();
    QString art = song.art_automatic();
    playlist_elem.appendChild(CreateSong(&doc, &song, &art, index));
    ++index;
  }

  root.appendChild(playlist_elem);

  SendDataToClients(doc.toByteArray());
}

void OutgoingXmlCreator::StateChanged(Engine::State state) {
  // Send state only if it changed
  // When selecting next song, StateChanged is emitted, but we already know
  // that we are playing
  if (state == last_state_) {
    return;
  }
  last_state_ = state;

  QDomDocument doc;
  QString action;

  switch (state) {
  case Engine::Playing: action = RemoteXmlTags::PLAY;
                        break;
  case Engine::Paused:  action = RemoteXmlTags::PAUSE;
                        break;
  case Engine::Empty:   action = RemoteXmlTags::STOP; // Empty is called when player stopped
                        break;
  default: break;
  };

  CreateXmlHeader(&doc, action);
  SendDataToClients(doc.toByteArray());
}

void OutgoingXmlCreator::CreateXmlHeader(QDomDocument *doc, QString action)
{
  // Create the header
  QDomProcessingInstruction xml_header = doc->createProcessingInstruction("xml", "version=\"1.0\"");
  doc->appendChild(xml_header);
  // Create the root
  QDomElement root = doc->createElement(RemoteXmlTags::ROOT);
  doc->appendChild(root);

  // Append the action
  QDomElement xml_action = doc->createElement(RemoteXmlTags::ACTION);
  QDomText text_action = doc->createTextNode(action);
  xml_action.appendChild(text_action);
  root.appendChild(xml_action);
}

void OutgoingXmlCreator::SendKeepAlive() {
  QDomDocument doc;
  CreateXmlHeader(&doc, RemoteXmlTags::KEEP_ALIVE);
  SendDataToClients(doc.toByteArray());
}
