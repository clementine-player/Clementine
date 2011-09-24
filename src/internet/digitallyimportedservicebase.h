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

class DigitallyImportedUrlHandler;

class QNetworkAccessManager;


class DigitallyImportedServiceBase : public InternetService {
  Q_OBJECT
  friend class DigitallyImportedUrlHandler;

public:
  DigitallyImportedServiceBase(
      const QString& name, const QString& description, const QUrl& homepage_url,
      const QString& homepage_name, const QUrl& stream_list_url,
      const QString& url_scheme, const QIcon& icon,
      InternetModel* model, QObject* parent = NULL);
  ~DigitallyImportedServiceBase();

  static const char* kSettingsGroup;
  static const int kStreamsCacheDurationSecs;

  QStandardItem* CreateRootItem();
  void LazyPopulate(QStandardItem* parent);
  void ShowContextMenu(const QModelIndex& index, const QPoint& global_pos);

  void ReloadSettings();

  bool is_valid_stream_selected() const;
  bool is_premium_stream_selected() const;
  bool is_premium_account() const;

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

protected:
  struct Playlist {
    Playlist(bool premium, const QString& url_template)
      : premium_(premium), url_template_(url_template) {}

    bool premium_;
    QString url_template_;
  };

  QModelIndex GetCurrentIndex();

  // Called by DigitallyImportedUrlHandler, implemented by subclasses, must
  // call LoadPlaylistFinished eventually.
  virtual void LoadStation(const QString& key) = 0;

protected slots:
  void LoadPlaylistFinished();

private slots:
  void Homepage();
  void ForceRefreshStreams();
  void RefreshStreams();
  void RefreshStreamsFinished();
  void ShowSettingsDialog();

protected:
  QNetworkAccessManager* network_;
  DigitallyImportedUrlHandler* url_handler_;

  int audio_type_;
  QString username_;
  QString password_;

  int task_id_;

  QList<Playlist> playlists_;

private:
  void PopulateStreams();
  StreamList LoadStreams() const;
  void SaveStreams(const StreamList& streams);

private:
  // Set by subclasses through the constructor
  QUrl homepage_url_;
  QString homepage_name_;
  QUrl stream_list_url_;
  QIcon icon_;
  QString service_description_;
  QString url_scheme_;

  QStandardItem* root_;

  QMenu* context_menu_;
  QStandardItem* context_item_;

  QList<Stream> saved_streams_;
  QDateTime last_refreshed_streams_;
};

#endif // DIGITALLYIMPORTEDSERVICEBASE_H
