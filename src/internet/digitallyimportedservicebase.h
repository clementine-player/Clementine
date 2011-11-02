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

#ifndef DIGITALLYIMPORTEDSERVICEBASE_H
#define DIGITALLYIMPORTEDSERVICEBASE_H

#include "internetservice.h"

class DigitallyImportedClient;
class DigitallyImportedUrlHandler;

class QNetworkAccessManager;


class DigitallyImportedServiceBase : public InternetService {
  Q_OBJECT
  friend class DigitallyImportedUrlHandler;

public:
  DigitallyImportedServiceBase(const QString& name, InternetModel* model,
                               QObject* parent = NULL);
  ~DigitallyImportedServiceBase();

  static const char* kSettingsGroup;
  static const int kStreamsCacheDurationSecs;

  QStandardItem* CreateRootItem();
  void LazyPopulate(QStandardItem* parent);
  void ShowContextMenu(const QModelIndex& index, const QPoint& global_pos);

  void ReloadSettings();

  bool is_premium_account() const;

  const QUrl& homepage_url() const { return homepage_url_; }
  const QString& homepage_name() const { return homepage_name_; }
  const QUrl& stream_list_url() const { return stream_list_url_; }
  const QString& icon_path() const { return icon_path_; }
  const QIcon& icon() const { return icon_; }
  const QString& service_description() const { return service_description_; }
  const QString& url_scheme() const { return url_scheme_; }
  const QString& api_service_name() const { return api_service_name_; }

  // Public for the global search provider.
  struct Stream {
    int id_;
    QString key_;
    QString name_;
    QString description_;

    bool operator <(const Stream& other) const { return name_ < other.name_; }
  };
  typedef QList<Stream> StreamList;

  bool IsStreamListStale() const;
  StreamList Streams();

signals:
  void StreamsChanged();

protected:
  // Subclasses must call this from their constructor
  void Init(const QString& description, const QUrl& homepage_url,
            const QString& homepage_name, const QUrl& stream_list_url,
            const QString& url_scheme, const QString& icon_path,
            const QString& api_service_name);

  QModelIndex GetCurrentIndex();

protected slots:
  void LoadPlaylistFinished();

private slots:
  void Homepage();
  void ForceRefreshStreams();
  void RefreshStreams();
  void RefreshStreamsFinished();
  void ShowSettingsDialog();

private:
  void PopulateStreams();
  StreamList LoadStreams() const;
  void SaveStreams(const StreamList& streams);

  void LoadStation(const QString& key);

private:
  // Set by subclasses through the constructor
  QUrl homepage_url_;
  QString homepage_name_;
  QUrl stream_list_url_;
  QString icon_path_;
  QIcon icon_;
  QString service_description_;
  QString url_scheme_;
  QString api_service_name_;

  QStringList basic_playlists_;
  QStringList premium_playlists_;

  QNetworkAccessManager* network_;
  DigitallyImportedUrlHandler* url_handler_;

  int basic_audio_type_;
  int premium_audio_type_;
  QString username_;
  QString listen_hash_;

  int task_id_;

  QStandardItem* root_;

  QMenu* context_menu_;
  QStandardItem* context_item_;

  QList<Stream> saved_streams_;
  QDateTime last_refreshed_streams_;

  DigitallyImportedClient* api_client_;
};

#endif // DIGITALLYIMPORTEDSERVICEBASE_H
