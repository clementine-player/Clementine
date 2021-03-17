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

#ifndef INTERNET_INTERGALACTICFM_INTERGALACTICFMSERVICE_H_
#define INTERNET_INTERGALACTICFM_INTERGALACTICFMSERVICE_H_

#include <QXmlStreamReader>

#include "core/cachedlist.h"
#include "internet/core/internetservice.h"

class IntergalacticFMUrlHandler;

class QNetworkAccessManager;
class QNetworkReply;
class QMenu;

class IntergalacticFMServiceBase : public InternetService {
  Q_OBJECT

 public:
  IntergalacticFMServiceBase(Application* app, InternetModel* parent,
                             const QString& name, const QUrl& channel_list_url,
                             const QUrl& homepage_url,
                             const QUrl& donate_page_url, const QIcon& icon);
  ~IntergalacticFMServiceBase();

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
  void ReadChannel(QXmlStreamReader& reader, StreamList* ret);
  void PopulateStreams();

 private:
  const QString url_scheme_;
  IntergalacticFMUrlHandler* url_handler_;

  QStandardItem* root_;

  QNetworkAccessManager* network_;

  CachedList<Stream> streams_;

  const QString name_;
  const QUrl channel_list_url_;
  const QUrl homepage_url_;
  const QUrl donate_page_url_;
  const QIcon icon_;
};

class IntergalacticFMService : public IntergalacticFMServiceBase {
 public:
  IntergalacticFMService(Application* app, InternetModel* parent);
};

QDataStream& operator<<(QDataStream& out,
                        const IntergalacticFMService::Stream& stream);
QDataStream& operator>>(QDataStream& in,
                        IntergalacticFMService::Stream& stream);
Q_DECLARE_METATYPE(IntergalacticFMService::Stream)

#endif  // INTERNET_INTERGALACTICFM_INTERGALACTICFMSERVICE_H_
