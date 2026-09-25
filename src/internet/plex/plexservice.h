/* This file is part of Clementine.
   Copyright 2026, John Maguire <john.maguire@gmail.com>

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

#ifndef INTERNET_PLEX_PLEXSERVICE_H_
#define INTERNET_PLEX_PLEXSERVICE_H_

#include <QDateTime>
#include <QHash>
#include <QUrlQuery>
#include <memory>

#include "internet/core/internetmodel.h"
#include "internet/core/internetservice.h"
#include "internet/plex/plexparser.h"

class QNetworkAccessManager;
class QNetworkReply;
class QNetworkRequest;
class QSortFilterProxyModel;
class QTimer;

class NetworkTimeouts;
class PlexLibraryScanner;
class PlexNotificationListener;
class PlexPlaybackReporter;
class PlexUrlHandler;

class PlexService : public InternetService {
  Q_OBJECT

 public:
  PlexService(Application* app, InternetModel* parent);
  ~PlexService() override;

  enum LoginState {
    LoginState_LoggedIn,
    LoginState_NotConfigured,
    LoginState_Connecting,
    LoginState_ServerUnreachable,
    LoginState_BadToken,
  };
  Q_ENUM(LoginState)

  static const char* kServiceName;
  static const char* kSettingsGroup;
  static const char* kSongsTable;
  static const char* kFtsTable;

  // The id Plex uses to tell this installation apart from other clients.
  // Generated on first use and kept in the settings.
  static QString ClientId();
  // Adds the X-Plex-* headers every Plex request needs.
  static void AddClientHeaders(QNetworkRequest* request, const QString& token);

  QStandardItem* CreateRootItem() override;
  void LazyPopulate(QStandardItem* item) override;
  void ShowContextMenu(const QPoint& global_pos) override;
  QWidget* HeaderWidget() const override;
  void ReloadSettings() override;

  bool ConfigRequired() override { return login_state_ != LoginState_LoggedIn; }

  LoginState login_state() const { return login_state_; }
  QString server_name() const { return server_.name; }
  QString machine_id() const { return server_.machine_id; }
  QString server_token() const { return server_.access_token; }
  QUrl base_url() const { return base_url_; }

  // GET a path on the connected server, e.g. "/library/sections".
  QNetworkReply* Get(const QString& path, const QUrlQuery& query = QUrlQuery());

  // Tells the service a track was just scrobbled, so the change notification
  // the server sends back doesn't trigger a sync.
  void NoteOwnChange(const QString& rating_key);

 signals:
  void LoginStateChanged(PlexService::LoginState state);

 public slots:
  void ShowConfig() override;
  // Re-fetches every music section whether or not the server says it changed.
  void ForceSync();

 private slots:
  void ProbeFinished(QNetworkReply* reply, int index, quint64 generation);
  void SectionsFinished(QNetworkReply* reply, bool force);
  void ScanProgress(int done, int total);
  void ScanFinished(const QMap<QString, SongList>& songs, bool ok);
  void LibraryChanged(const QStringList& item_ids);
  void DebouncedSync();

 private:
  struct Probe {
    QUrl uri;
    bool finished = false;
    bool ok = false;
    int http_status = 0;
    QByteArray body;
  };

  void SetLoginState(LoginState state);
  void Connect();
  void CheckProbes();
  QNetworkRequest MakeRequest(const QUrl& url) const;
  // Fetches /library/sections and syncs the sections that changed.
  void RequestSync(bool force);
  void StartSync(const QList<PlexSection>& sections, bool force);
  void ClearLibrary();

  QNetworkAccessManager* network_;
  NetworkTimeouts* probe_timeouts_;
  PlexUrlHandler* url_handler_;
  PlexLibraryScanner* scanner_;
  PlexNotificationListener* listener_;
  PlexPlaybackReporter* reporter_;
  QTimer* sync_debounce_;

  QStandardItem* root_;
  std::shared_ptr<LibraryBackend> library_backend_;
  LibraryModel* library_model_;
  LibraryFilterWidget* library_filter_;
  QSortFilterProxyModel* library_sort_model_;
  int sync_task_id_;

  PlexServer server_;
  bool verify_cert_;
  LoginState login_state_;
  QUrl base_url_;

  QList<Probe> probes_;
  // Bumped on every Connect() so replies from older probes are ignored.
  quint64 probe_generation_;

  // Sections being scanned, with their new change stamps.  Only written back
  // to the settings once the scan has succeeded.
  QList<PlexSection> sections_;
  QStringList syncing_sections_;
  bool pending_force_sync_;

  // Rating keys we changed ourselves (scrobbles), and when.
  QHash<QString, QDateTime> own_changes_;
};

#endif  // INTERNET_PLEX_PLEXSERVICE_H_
