/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

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

#ifndef SOMAFMSERVICE_H
#define SOMAFMSERVICE_H

#include <QXmlStreamReader>

#include "internetservice.h"
#include "core/cachedlist.h"

class SomaFMUrlHandler;

class QNetworkAccessManager;
class QNetworkReply;
class QMenu;

class SomaFMService : public InternetService {
  Q_OBJECT

public:
  SomaFMService(Application* app, InternetModel* parent);
  ~SomaFMService();

  enum ItemType {
    Type_Stream = 2000,
  };

  struct Stream {
    QString title_;
    QString dj_;
    QUrl url_;

    Song ToSong() const;
  };
  typedef QList<Stream> StreamList;

  static const char* kServiceName;
  static const char* kSettingsGroup;
  static const char* kChannelListUrl;
  static const char* kHomepage;
  static const int kStreamsCacheDurationSecs;

  QStandardItem* CreateRootItem();
  void LazyPopulate(QStandardItem* item);
  void ShowContextMenu(const QPoint& global_pos);

  PlaylistItem::Options playlistitem_options() const;
  QNetworkAccessManager* network() const { return network_; }

  void ReloadSettings();

  bool IsStreamListStale() const { return streams_.IsStale(); }
  StreamList Streams();

signals:
  void StreamsChanged();

private slots:
  void ForceRefreshStreams();
  void RefreshStreams();
  void RefreshStreamsFinished(QNetworkReply* reply, int task_id);

  void Homepage();

private:
  void ReadChannel(QXmlStreamReader& reader, StreamList* ret);
  void PopulateStreams();

private:
  SomaFMUrlHandler* url_handler_;

  QStandardItem* root_;
  QMenu* context_menu_;

  QNetworkAccessManager* network_;

  CachedList<Stream> streams_;
};

QDataStream& operator<<(QDataStream& out, const SomaFMService::Stream& stream);
QDataStream& operator>>(QDataStream& in, SomaFMService::Stream& stream);
Q_DECLARE_METATYPE(SomaFMService::Stream)

#endif // SOMAFMSERVICE_H
