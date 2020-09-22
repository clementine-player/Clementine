/* This file is part of Clementine.
   Copyright 2015, Nick Lanham <nick@afternight.org>

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

#ifndef INTERNET_SUBSONIC_SUBSONICDYNAMICPLAYLIST_H_
#define INTERNET_SUBSONIC_SUBSONICDYNAMICPLAYLIST_H_

#include <QNetworkAccessManager>
#include <QXmlStreamReader>

#include "smartplaylists/generator.h"

class SubsonicService;

class SubsonicDynamicPlaylist : public smart_playlists::Generator {
  Q_OBJECT
  friend QDataStream& operator<<(QDataStream& s,
                                 const SubsonicDynamicPlaylist& p);
  friend QDataStream& operator>>(QDataStream& s, SubsonicDynamicPlaylist& p);

 public:
  // things that subsonic can return to us, persisted so only add at end
  enum QueryStat {
    QueryStat_Newest = 0,
    QueryStat_Highest = 1,
    QueryStat_Frequent = 2,
    QueryStat_Recent = 3,
    QueryStat_Starred = 4,
    QueryStat_Random = 5,
  };

  enum QueryType {
    QueryType_Album = 0,
    QueryType_Song = 1,
  };

  SubsonicDynamicPlaylist();
  SubsonicDynamicPlaylist(const QString& name, QueryType type, QueryStat stat);

  QString type() const { return "Subsonic"; }

  void Load(const QByteArray& data);
  void Load(QueryStat stat);
  QByteArray Save() const;

  PlaylistItemList Generate();

  bool is_dynamic() const { return true; }
  PlaylistItemList GenerateMoreAlbums(int count);
  PlaylistItemList GenerateMoreSongs(int count);

  static const int kMaxCount;
  static const int kDefaultAlbumCount;
  static const int kDefaultSongCount;
  static const int kDefaultOffset;

 private:
  void GetAlbum(PlaylistItemList& list, QString id,
                QNetworkAccessManager& network, const bool usesslv3);
  // need our own one since we run in a different thread from service
  QNetworkReply* Send(QNetworkAccessManager& network, const QUrl& url,
                      const bool usesslv3);
  QString GetTypeString() const {
    switch (stat_) {
      case QueryStat::QueryStat_Newest:
        return "newest";
      case QueryStat::QueryStat_Highest:
        return "highest";
      case QueryStat::QueryStat_Frequent:
        return "frequent";
      case QueryStat::QueryStat_Recent:
        return "recent";
      case QueryStat::QueryStat_Starred:
        return "starred";
      case QueryStat::QueryStat_Random:
        return "random";
      default:
        return "newest";
    }
  }

 private:
  QueryType type_;
  QueryStat stat_;
  int offset_;
  SubsonicService* service_;
};

QDataStream& operator<<(QDataStream& s, const SubsonicDynamicPlaylist& p);
QDataStream& operator>>(QDataStream& s, SubsonicDynamicPlaylist& p);

#endif  // INTERNET_SUBSONIC_SUBSONICDYNAMICPLAYLIST_H_
