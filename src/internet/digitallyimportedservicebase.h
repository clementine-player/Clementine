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

#include <memory>

#include "digitallyimportedclient.h"
#include "internetservice.h"
#include "core/cachedlist.h"

class DigitallyImportedClient;
class DigitallyImportedUrlHandler;

class QNetworkAccessManager;


class DigitallyImportedServiceBase : public InternetService {
  Q_OBJECT
  friend class DigitallyImportedUrlHandler;

public:
  DigitallyImportedServiceBase(const QString& name,
                               const QString& description,
                               const QUrl& homepage_url,
                               const QIcon& icon,
                               const QString& api_service_name,
                               Application* app,
                               InternetModel* model,
                               QObject* parent = NULL);
  ~DigitallyImportedServiceBase();

  static const char* kSettingsGroup;
  static const int kStreamsCacheDurationSecs;

  QStandardItem* CreateRootItem();
  void LazyPopulate(QStandardItem* parent);
  void ShowContextMenu(const QPoint& global_pos);

  void ReloadSettings();

  bool is_premium_account() const;

  const QUrl& homepage_url() const { return homepage_url_; }
  const QIcon& icon() const { return icon_; }
  const QString& service_description() const { return service_description_; }
  const QString& api_service_name() const { return api_service_name_; }

  bool IsChannelListStale() const { return saved_channels_.IsStale(); }
  DigitallyImportedClient::ChannelList Channels();
  void SongFromChannel(const DigitallyImportedClient::Channel& channel,
                       Song* song) const;

public slots:
  void ShowSettingsDialog();

signals:
  void StreamsChanged();

private slots:
  void LoadPlaylistFinished(QNetworkReply* reply);
  void Homepage();
  void ForceRefreshStreams();
  void RefreshStreams();
  void RefreshStreamsFinished(QNetworkReply* reply, int task_id);

private:
  void PopulateStreams();

  void LoadStation(const QString& key);

private:
  // Set by subclasses through the constructor
  QUrl homepage_url_;
  QIcon icon_;
  QString service_description_;
  QString api_service_name_;

  QStringList basic_playlists_;
  QStringList premium_playlists_;

  QNetworkAccessManager* network_;
  DigitallyImportedUrlHandler* url_handler_;

  int basic_audio_type_;
  int premium_audio_type_;
  QString username_;
  QString listen_hash_;

  QStandardItem* root_;

  std::unique_ptr<QMenu> context_menu_;
  QStandardItem* context_item_;

  CachedList<DigitallyImportedClient::Channel> saved_channels_;

  DigitallyImportedClient* api_client_;
};

class DigitallyImportedService : public DigitallyImportedServiceBase {
public:
  DigitallyImportedService(Application* app, InternetModel* model, QObject* parent = NULL);
};

class SkyFmService : public DigitallyImportedServiceBase {
public:
  SkyFmService(Application* app, InternetModel* model, QObject* parent = NULL);
};

class JazzRadioService : public DigitallyImportedServiceBase {
public:
  JazzRadioService(Application* app, InternetModel* model, QObject* parent = NULL);
};

class RockRadioService : public DigitallyImportedServiceBase {
public:
  RockRadioService(Application* app, InternetModel* model, QObject* parent = NULL);
};

#endif // DIGITALLYIMPORTEDSERVICEBASE_H
