/* This file is part of Clementine.
   Copyright 2010-2012, David Sansome <me@davidsansome.com>
   Copyright 2011, Tyler Rhodes <tyler.s.rhodes@gmail.com>
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

#ifndef INTERNET_MAGNATUNESERVICE_H_
#define INTERNET_MAGNATUNESERVICE_H_

#include <QXmlStreamReader>

#include "internet/core/internetservice.h"

class QNetworkAccessManager;
class QSortFilterProxyModel;
class QMenu;

class LibraryBackend;
class LibraryModel;
class MagnatuneUrlHandler;

class MagnatuneService : public InternetService {
  Q_OBJECT

 public:
  MagnatuneService(Application* app, InternetModel* parent);
  ~MagnatuneService();

  // Values are saved in QSettings and are indices into the combo box in
  // MagnatuneConfig
  enum MembershipType {
    Membership_None = 0,
    Membership_Streaming = 1,
    Membership_Download = 2,
  };

  // Values are saved in QSettings and are indices into the combo box in
  // MagnatuneConfig
  enum PreferredFormat {
    Format_Ogg = 0,
    Format_Flac = 1,
    Format_Wav = 2,
    Format_MP3_VBR = 3,
    Format_MP3_128 = 4,
  };

  static const char* kServiceName;
  static const char* kSettingsGroup;
  static const char* kDatabaseUrl;
  static const char* kSongsTable;
  static const char* kFtsTable;
  static const char* kHomepage;
  static const char* kStreamingHostname;
  static const char* kDownloadHostname;
  static const char* kPartnerId;
  static const char* kDownloadUrl;

  static QString ReadElementText(QXmlStreamReader& reader);

  QStandardItem* CreateRootItem();
  void LazyPopulate(QStandardItem* item);

  void ShowContextMenu(const QPoint& global_pos);

  QWidget* HeaderWidget() const;

  void ReloadSettings();

  // Magnatune specific stuff
  MembershipType membership_type() const { return membership_; }
  PreferredFormat preferred_format() const { return format_; }
  QString username() const { return username_; }
  QString password() const { return password_; }
  LibraryBackend* library_backend() const { return library_backend_; }

  QUrl ModifyUrl(const QUrl& url) const;

 signals:
  void DownloadFinished(const QStringList& albums);

 private slots:
  void UpdateTotalSongCount(int count);
  void ReloadDatabase();
  void ReloadDatabaseFinished();

  void Download();
  void Homepage();
  void ShowConfig();

 private:
  void EnsureMenuCreated();

  Song ReadTrack(QXmlStreamReader& reader);

 private:
  MagnatuneUrlHandler* url_handler_;

  QMenu* context_menu_;
  QStandardItem* root_;

  QAction* download_;

  LibraryBackend* library_backend_;
  LibraryModel* library_model_;
  LibraryFilterWidget* library_filter_;
  QSortFilterProxyModel* library_sort_model_;
  int load_database_task_id_;

  MembershipType membership_;
  QString username_;
  QString password_;
  PreferredFormat format_;

  int total_song_count_;

  QNetworkAccessManager* network_;
};

#endif  // INTERNET_MAGNATUNESERVICE_H_
