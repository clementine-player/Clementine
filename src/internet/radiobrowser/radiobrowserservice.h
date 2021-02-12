/* This file is part of Clementine.
   Copyright 2010-2013, David Sansome <me@davidsansome.com>
   Copyright 2010, 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>

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

#ifndef INTERNET_RADIOBROWSER_RADIOBROWSERSERVICE_H_
#define INTERNET_RADIOBROWSER_RADIOBROWSERSERVICE_H_

#include <QJsonObject>

#include "core/cachedlist.h"
#include "internet/core/internetservice.h"

class RadioBrowserUrlHandler;

class QNetworkAccessManager;
class QNetworkReply;
class QMenu;

class RadioBrowserServiceBase : public InternetService {
  Q_OBJECT

 public:
  RadioBrowserServiceBase(Application* app, InternetModel* parent,
                             const QString& name, const QUrl& channel_list_url,
                             const QUrl& homepage_url,
                             const QUrl& donate_page_url, const QIcon& icon);
  ~RadioBrowserServiceBase();

  enum ItemType {
    Type_Stream = 2000,
  };

  struct Stream {
    QString name_;
    QUrl url_;

    Song ToSong(const QString& prefix) const;
  };
  typedef QList<Stream> StreamList;

  static const int kStreamsCacheDurationSecs;

  const QString& url_scheme() const { return url_scheme_; }
  const QIcon& icon() const { return icon_; }

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
  void Donate();

 private:
  void ReadStation(QJsonObject& value, StreamList* ret);
  void PopulateStreams();

 private:
  const QString url_scheme_;
  RadioBrowserUrlHandler* url_handler_;

  QStandardItem* root_;
  QMenu* context_menu_;

  QNetworkAccessManager* network_;

  CachedList<Stream> streams_;

  const QString name_;
  const QUrl channel_list_url_;
  const QUrl homepage_url_;
  const QUrl donate_page_url_;
  const QIcon icon_;
};

class RadioBrowserService : public RadioBrowserServiceBase {
 public:
  RadioBrowserService(Application* app, InternetModel* parent);
};

QDataStream& operator<<(QDataStream& out,
                        const RadioBrowserService::Stream& stream);
QDataStream& operator>>(QDataStream& in,
                        RadioBrowserService::Stream& stream);
Q_DECLARE_METATYPE(RadioBrowserService::Stream)

#endif  // INTERNET_RADIOBROWSER_RADIOBROWSERSERVICE_H_
