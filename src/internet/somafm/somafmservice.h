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

#ifndef INTERNET_SOMAFM_SOMAFMSERVICE_H_
#define INTERNET_SOMAFM_SOMAFMSERVICE_H_

#include <QXmlStreamReader>

#include "core/cachedlist.h"
#include "internet/core/internetservice.h"

class SomaFMUrlHandler;

class QNetworkAccessManager;
class QNetworkReply;
class QMenu;

class SomaFMServiceBase : public InternetService {
  Q_OBJECT

 public:
  SomaFMServiceBase(Application* app, InternetModel* parent,
                    const QString& name, const QUrl& channel_list_url,
                    const QUrl& homepage_url, const QUrl& donate_page_url,
                    const QIcon& icon);
  ~SomaFMServiceBase() override;

  enum ItemType {
    Type_Stream = 2000,
  };

  struct Stream {
    QString title_;
    QString dj_;
    QUrl url_;

    Song ToSong(const QString& prefix) const;
  };
  typedef QList<Stream> StreamList;

  static const int kStreamsCacheDurationSecs;

  const QString& url_scheme() const { return url_scheme_; }
  const QIcon& icon() const { return icon_; }

  QStandardItem* CreateRootItem() override;
  void LazyPopulate(QStandardItem* item) override;
  void ShowContextMenu(const QPoint& global_pos) override;

  PlaylistItem::Options playlistitem_options() const override;
  QNetworkAccessManager* network() const { return network_; }

  void ReloadSettings() override;

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
  void ReadChannel(QXmlStreamReader& reader, StreamList* ret);
  void PopulateStreams();

 private:
  const QString url_scheme_;
  SomaFMUrlHandler* url_handler_;

  QStandardItem* root_;

  QNetworkAccessManager* network_;

  CachedList<Stream> streams_;

  const QString name_;
  const QUrl channel_list_url_;
  const QUrl homepage_url_;
  const QUrl donate_page_url_;
  const QIcon icon_;
};

class SomaFMService : public SomaFMServiceBase {
 public:
  SomaFMService(Application* app, InternetModel* parent);
};

QDataStream& operator<<(QDataStream& out, const SomaFMService::Stream& stream);
QDataStream& operator>>(QDataStream& in, SomaFMService::Stream& stream);
Q_DECLARE_METATYPE(SomaFMService::Stream)

#endif  // INTERNET_SOMAFM_SOMAFMSERVICE_H_
