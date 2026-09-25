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

#include "plexservice.h"

#include <QMenu>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSet>
#include <QSettings>
#include <QSortFilterProxyModel>
#include <QSslConfiguration>
#include <QSysInfo>
#include <QTimer>
#include <QUuid>

#include "config.h"
#include "core/application.h"
#include "core/closure.h"
#include "core/database.h"
#include "core/logging.h"
#include "core/mergedproxymodel.h"
#include "core/network.h"
#include "core/player.h"
#include "core/taskmanager.h"
#include "core/utilities.h"
#include "globalsearch/globalsearch.h"
#include "globalsearch/librarysearchprovider.h"
#include "internet/plex/plexlibraryscanner.h"
#include "internet/plex/plexnotificationlistener.h"
#include "internet/plex/plexplaybackreporter.h"
#include "internet/plex/plexurlhandler.h"
#include "library/librarybackend.h"
#include "library/libraryfilterwidget.h"
#include "library/librarymodel.h"
#include "ui/iconloader.h"
#include "version.h"

const char* PlexService::kServiceName = "Plex";
const char* PlexService::kSettingsGroup = "Plex";
const char* PlexService::kSongsTable = "plex_songs";
const char* PlexService::kFtsTable = "plex_songs_fts";

namespace {

const int kProbeTimeoutMsec = 5000;
// Long enough for one Plex scan to settle into a single sync.
const int kSyncDebounceMsec = 30000;
// How long a scrobble's echo from the server is ignored for.
const int kOwnChangeSecs = 120;

}  // namespace

PlexService::PlexService(Application* app, InternetModel* parent)
    : InternetService(kServiceName, app, parent, parent),
      network_(new QNetworkAccessManager(this)),
      probe_timeouts_(new NetworkTimeouts(kProbeTimeoutMsec, this)),
      url_handler_(new PlexUrlHandler(this, this)),
      scanner_(new PlexLibraryScanner(this, this)),
      listener_(new PlexNotificationListener(this, this)),
      reporter_(new PlexPlaybackReporter(this, app, this)),
      sync_debounce_(new QTimer(this)),
      root_(nullptr),
      library_model_(nullptr),
      library_filter_(nullptr),
      library_sort_model_(new QSortFilterProxyModel(this)),
      sync_task_id_(0),
      verify_cert_(true),
      login_state_(LoginState_NotConfigured),
      probe_generation_(0),
      pending_force_sync_(false) {
  app_->player()->RegisterUrlHandler(url_handler_);

  connect(scanner_, SIGNAL(Progress(int, int)), SLOT(ScanProgress(int, int)));
  connect(scanner_, SIGNAL(ScanFinished(QMap<QString, SongList>, bool)),
          SLOT(ScanFinished(QMap<QString, SongList>, bool)));
  connect(listener_, SIGNAL(LibraryChanged(QStringList)),
          SLOT(LibraryChanged(QStringList)));

  sync_debounce_->setSingleShot(true);
  sync_debounce_->setInterval(kSyncDebounceMsec);
  connect(sync_debounce_, SIGNAL(timeout()), SLOT(DebouncedSync()));

  library_backend_.reset(new LibraryBackend,
                         [](QObject* obj) { obj->deleteLater(); });
  library_backend_->moveToThread(app_->database()->thread());
  library_backend_->Init(app_->database(), kSongsTable, kFtsTable);

  library_model_ = new LibraryModel(library_backend_, app_, this);
  library_model_->set_show_various_artists(false);
  library_model_->set_show_smart_playlists(false);

  library_filter_ = new LibraryFilterWidget(0);
  library_filter_->SetSettingsGroup(kSettingsGroup);
  library_filter_->SetLibraryModel(library_model_);
  library_filter_->SetFilterHint(tr("Search Plex"));
  library_filter_->SetAgeFilterEnabled(false);

  library_sort_model_->setSourceModel(library_model_);
  library_sort_model_->setSortRole(LibraryModel::Role_SortText);
  library_sort_model_->setDynamicSortFilter(true);
  library_sort_model_->setSortLocaleAware(true);
  library_sort_model_->sort(0);

  context_menu_.reset(new QMenu);
  context_menu_->addActions(GetPlaylistActions());
  context_menu_->addSeparator();
  context_menu_->addAction(IconLoader::Load("view-refresh", IconLoader::Base),
                           tr("Refresh catalogue"), this, SLOT(ForceSync()));
  QAction* config_action = context_menu_->addAction(
      IconLoader::Load("configure", IconLoader::Base), tr("Configure Plex..."),
      this, SLOT(ShowConfig()));
  context_menu_->addSeparator();
  context_menu_->addMenu(library_filter_->menu());

  library_filter_->AddMenuAction(config_action);

  app_->global_search()->AddProvider(new LibrarySearchProvider(
      library_backend_.get(), tr("Plex"), "plex",
      IconLoader::Load("plex", IconLoader::Provider), true, app_, this));
}

PlexService::~PlexService() {}

QString PlexService::ClientId() {
  QSettings s;
  s.beginGroup(kSettingsGroup);
  QString id = s.value("client_id").toString();
  if (id.isEmpty()) {
    id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    s.setValue("client_id", id);
  }
  return id;
}

void PlexService::AddClientHeaders(QNetworkRequest* request,
                                   const QString& token) {
  request->setRawHeader("Accept", "application/json");
  request->setRawHeader("X-Plex-Client-Identifier", ClientId().toUtf8());
  request->setRawHeader("X-Plex-Product", "Clementine");
  request->setRawHeader("X-Plex-Version", CLEMENTINE_VERSION_DISPLAY);
  request->setRawHeader("X-Plex-Platform", QSysInfo::productType().toUtf8());
  request->setRawHeader("X-Plex-Platform-Version",
                        QSysInfo::productVersion().toUtf8());
  request->setRawHeader("X-Plex-Device-Name",
                        QSysInfo::machineHostName().toUtf8());
  if (!token.isEmpty()) request->setRawHeader("X-Plex-Token", token.toUtf8());
}

QStandardItem* PlexService::CreateRootItem() {
  root_ = new QStandardItem(IconLoader::Load("plex", IconLoader::Provider),
                            kServiceName);
  root_->setData(true, InternetModel::Role_CanLazyLoad);
  return root_;
}

void PlexService::LazyPopulate(QStandardItem* item) {
  switch (item->data(InternetModel::Role_Type).toInt()) {
    case InternetModel::Type_Service:
      library_model_->Init();
      if (login_state_ == LoginState_NotConfigured) {
        ShowConfig();
      }
      model()->merged_model()->AddSubModel(item->index(), library_sort_model_);
      break;

    default:
      break;
  }
}

void PlexService::ShowContextMenu(const QPoint& global_pos) {
  const bool is_valid = model()->current_index().model() == library_sort_model_;

  GetAppendToPlaylistAction()->setEnabled(is_valid);
  GetReplacePlaylistAction()->setEnabled(is_valid);
  GetOpenInNewPlaylistAction()->setEnabled(is_valid);
  context_menu_->popup(global_pos);
}

QWidget* PlexService::HeaderWidget() const { return library_filter_; }

void PlexService::ShowConfig() {
  app_->OpenSettingsDialogAtPage(SettingsDialog::Page_Plex);
}

void PlexService::ReloadSettings() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  PlexServer server;
  server.machine_id = s.value("server_id").toString();
  server.name = s.value("server_name").toString();
  server.access_token = s.value("server_token").toString();
  server.connections =
      PlexParser::ParseConnections(s.value("connections").toByteArray());
  const bool verify_cert = s.value("verify_cert", true).toBool();

  // Settings are reloaded whenever any page is saved.  Don't drop a working
  // connection if nothing Plex-related changed.
  if (login_state_ == LoginState_LoggedIn &&
      server.machine_id == server_.machine_id &&
      server.access_token == server_.access_token &&
      verify_cert == verify_cert_) {
    server_.name = server.name;
    return;
  }

  listener_->Stop();
  base_url_ = QUrl();
  server_ = server;
  verify_cert_ = verify_cert;

  if (server_.machine_id.isEmpty() || server_.access_token.isEmpty()) {
    ClearLibrary();
    SetLoginState(LoginState_NotConfigured);
    return;
  }

  // The local table only ever holds one server's tracks.
  if (s.value("synced_server_id").toString() != server_.machine_id) {
    ClearLibrary();
    s.setValue("synced_server_id", server_.machine_id);
  }

  Connect();
}

void PlexService::SetLoginState(LoginState state) {
  if (state == login_state_) return;
  login_state_ = state;
  qLog(Debug) << "Plex login state changed:"
              << Utilities::EnumToString(staticMetaObject, "LoginState", state);
  emit LoginStateChanged(state);
}

QNetworkRequest PlexService::MakeRequest(const QUrl& url) const {
  QNetworkRequest request(url);
  AddClientHeaders(&request, server_.access_token);
  QSslConfiguration ssl = QSslConfiguration::defaultConfiguration();
  ssl.setPeerVerifyMode(verify_cert_ ? QSslSocket::VerifyPeer
                                     : QSslSocket::VerifyNone);
  request.setSslConfiguration(ssl);
  return request;
}

QNetworkReply* PlexService::Get(const QString& path, const QUrlQuery& query) {
  QUrl url(base_url_.toString() + path);
  if (!query.isEmpty()) url.setQuery(query);
  return network_->get(MakeRequest(url));
}

void PlexService::Connect() {
  SetLoginState(LoginState_Connecting);

  // Try every connection at once, but prefer them in the order
  // OrderConnections gives: a local server beats a remote one, even if the
  // remote one answers first.
  ++probe_generation_;
  probes_.clear();
  for (const PlexConnection& connection :
       PlexParser::OrderConnections(server_.connections)) {
    Probe probe;
    probe.uri = connection.uri;
    probes_ << probe;
  }

  if (probes_.isEmpty()) {
    SetLoginState(LoginState_ServerUnreachable);
    return;
  }

  // /library/sections checks the token as well as reachability, and its
  // answer is needed to sync anyway.
  for (int i = 0; i < probes_.size(); ++i) {
    QUrl url(probes_[i].uri.toString() + "/library/sections");
    QNetworkReply* reply = network_->get(MakeRequest(url));
    probe_timeouts_->AddReply(reply);
    NewClosure(reply, &QNetworkReply::finished, this,
               &PlexService::ProbeFinished, reply, i, probe_generation_);
  }
}

void PlexService::ProbeFinished(QNetworkReply* reply, int index,
                                quint64 generation) {
  reply->deleteLater();
  if (generation != probe_generation_) return;

  Probe& probe = probes_[index];
  probe.finished = true;
  probe.http_status =
      reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
  probe.ok = reply->error() == QNetworkReply::NoError;
  if (probe.ok) {
    probe.body = reply->readAll();
  } else {
    qLog(Debug) << "Plex connection" << probe.uri
                << "failed:" << reply->errorString();
  }

  CheckProbes();
}

void PlexService::CheckProbes() {
  bool bad_token = false;
  for (const Probe& probe : probes_) {
    // Wait for any better connection that's still being tried.
    if (!probe.finished) return;

    if (probe.ok) {
      ++probe_generation_;  // Ignore the rest.
      base_url_ = probe.uri;
      qLog(Info) << "Connected to Plex server" << server_.name << "at"
                 << base_url_;
      SetLoginState(LoginState_LoggedIn);

      const QList<PlexSection> sections =
          PlexParser::ParseMusicSections(QJsonDocument::fromJson(probe.body));
      probes_.clear();
      StartSync(sections, false);
      listener_->Start();
      return;
    }

    if (probe.http_status == 401) bad_token = true;
  }

  probes_.clear();
  SetLoginState(bad_token ? LoginState_BadToken : LoginState_ServerUnreachable);
}

void PlexService::ForceSync() {
  if (login_state_ != LoginState_LoggedIn) return;
  RequestSync(true);
}

void PlexService::RequestSync(bool force) {
  QNetworkReply* reply = Get("/library/sections");
  NewClosure(reply, &QNetworkReply::finished, this,
             &PlexService::SectionsFinished, reply, force);
}

void PlexService::SectionsFinished(QNetworkReply* reply, bool force) {
  reply->deleteLater();
  QJsonDocument doc = ParseJsonReply(reply);
  if (doc.isNull()) return;

  StartSync(PlexParser::ParseMusicSections(doc), force);
}

void PlexService::StartSync(const QList<PlexSection>& sections, bool force) {
  if (scanner_->is_scanning()) {
    // Run again once this scan is done.
    pending_force_sync_ |= force;
    sync_debounce_->start();
    return;
  }

  QSettings s;
  s.beginGroup(kSettingsGroup);
  const QVariantMap stamps = s.value("section_stamps").toMap();

  sections_ = sections;
  QStringList changed;
  QSet<QString> current_keys;
  for (const PlexSection& section : sections) {
    current_keys << section.key;
    if (force || stamps.value(section.key).toLongLong() != section.changed_at) {
      changed << section.key;
    }
  }

  // Drop the tracks of any section that has gone from the server.
  bool section_removed = false;
  for (const QString& key : stamps.keys()) {
    if (!current_keys.contains(key)) section_removed = true;
  }
  if (section_removed) {
    SongList orphans;
    for (const Song& song : library_backend_->GetAllSongs()) {
      if (!current_keys.contains(PlexParser::SectionKeyFromUrl(song.url()))) {
        orphans << song;
      }
    }
    library_backend_->DeleteSongs(orphans);

    QVariantMap kept;
    for (const QString& key : current_keys) {
      if (stamps.contains(key)) kept[key] = stamps[key];
    }
    s.setValue("section_stamps", kept);
  }

  if (changed.isEmpty()) {
    qLog(Debug) << "Plex library is up to date";
    return;
  }

  qLog(Debug) << "Syncing Plex sections" << changed;
  syncing_sections_ = changed;
  if (!sync_task_id_) {
    sync_task_id_ =
        app_->task_manager()->StartTask(tr("Fetching Plex library"));
  }
  scanner_->Scan(changed);
}

void PlexService::ScanProgress(int done, int total) {
  app_->task_manager()->SetTaskProgress(sync_task_id_, done, total);
}

void PlexService::ScanFinished(const QMap<QString, SongList>& songs, bool ok) {
  app_->task_manager()->SetTaskFinished(sync_task_id_);
  sync_task_id_ = 0;

  if (!ok) {
    // Keep what we have rather than deleting tracks we failed to fetch.
    qLog(Warning) << "Plex sync failed; keeping the existing library";
    return;
  }

  const SongList existing = library_backend_->GetAllSongs();

  QSettings s;
  s.beginGroup(kSettingsGroup);
  QVariantMap stamps = s.value("section_stamps").toMap();

  SongList add_or_update;
  SongList remove;
  for (auto it = songs.constBegin(); it != songs.constEnd(); ++it) {
    SongList existing_in_section;
    for (const Song& song : existing) {
      if (PlexParser::SectionKeyFromUrl(song.url()) == it.key()) {
        existing_in_section << song;
      }
    }

    PlexDiff diff = PlexParser::ComputeDiff(existing_in_section, it.value());
    qLog(Debug) << "Plex section" << it.key() << ":" << diff.add.size()
                << "added," << diff.update.size() << "updated,"
                << diff.remove.size() << "removed";
    add_or_update << diff.add << diff.update;
    remove << diff.remove;

    for (const PlexSection& section : sections_) {
      if (section.key == it.key()) stamps[section.key] = section.changed_at;
    }
  }

  if (!remove.isEmpty()) library_backend_->DeleteSongs(remove);
  if (!add_or_update.isEmpty())
    library_backend_->AddOrUpdateSongs(add_or_update);
  s.setValue("section_stamps", stamps);
}

void PlexService::NoteOwnChange(const QString& rating_key) {
  own_changes_[rating_key] = QDateTime::currentDateTimeUtc();
}

void PlexService::LibraryChanged(const QStringList& item_ids) {
  const QDateTime now = QDateTime::currentDateTimeUtc();
  for (auto it = own_changes_.begin(); it != own_changes_.end();) {
    if (it->secsTo(now) > kOwnChangeSecs) {
      it = own_changes_.erase(it);
    } else {
      ++it;
    }
  }

  if (item_ids.isEmpty()) {
    // Something like a library scan finished.  The section stamps say
    // whether a music section was affected.
    sync_debounce_->start();
    return;
  }

  for (const QString& id : item_ids) {
    if (!own_changes_.contains(id)) {
      // Item edits don't always move the section stamps, so fetch
      // everything.
      pending_force_sync_ = true;
      sync_debounce_->start();
      return;
    }
  }
}

void PlexService::DebouncedSync() {
  if (login_state_ != LoginState_LoggedIn) return;
  const bool force = pending_force_sync_;
  pending_force_sync_ = false;
  RequestSync(force);
}

void PlexService::ClearLibrary() {
  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.remove("section_stamps");
  library_backend_->DeleteAll();
}
