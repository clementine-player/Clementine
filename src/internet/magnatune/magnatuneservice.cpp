/* This file is part of Clementine.
   Copyright 2010-2013, David Sansome <me@davidsansome.com>
   Copyright 2011, Arnaud Bienner <arnaud.bienner@gmail.com>
   Copyright 2011, Tyler Rhodes <tyler.s.rhodes@gmail.com>
   Copyright 2011, Paweł Bara <keirangtp@gmail.com>
   Copyright 2012, 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2013, Alan Briolat <alan.briolat@gmail.com>
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

#include "magnatuneservice.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QXmlStreamReader>
#include <QSortFilterProxyModel>
#include <QMenu>
#include <QDesktopServices>
#include <QCoreApplication>
#include <QSettings>

#include <QtDebug>

#include "qtiocompressor.h"


#include "magnatunedownloaddialog.h"
#include "magnatuneplaylistitem.h"
#include "magnatuneurlhandler.h"
#include "internet/core/internetmodel.h"
#include "core/application.h"
#include "core/database.h"
#include "core/logging.h"
#include "core/mergedproxymodel.h"
#include "core/network.h"
#include "core/player.h"
#include "core/song.h"
#include "core/taskmanager.h"
#include "core/timeconstants.h"
#include "globalsearch/globalsearch.h"
#include "globalsearch/librarysearchprovider.h"
#include "library/librarymodel.h"
#include "library/librarybackend.h"
#include "library/libraryfilterwidget.h"
#include "ui/iconloader.h"
#include "ui/settingsdialog.h"

const char* MagnatuneService::kServiceName = "Magnatune";
const char* MagnatuneService::kSettingsGroup = "Magnatune";
const char* MagnatuneService::kSongsTable = "magnatune_songs";
const char* MagnatuneService::kFtsTable = "magnatune_songs_fts";

const char* MagnatuneService::kHomepage = "http://magnatune.com";
const char* MagnatuneService::kDatabaseUrl =
    "http://magnatune.com/info/song_info_xml.gz";
const char* MagnatuneService::kStreamingHostname = "streaming.magnatune.com";
const char* MagnatuneService::kDownloadHostname = "download.magnatune.com";

const char* MagnatuneService::kPartnerId = "clementine";
const char* MagnatuneService::kDownloadUrl =
    "http://download.magnatune.com/buy/membership_free_dl_xml";

MagnatuneService::MagnatuneService(Application* app, InternetModel* parent)
    : InternetService(kServiceName, app, parent, parent),
      url_handler_(new MagnatuneUrlHandler(this, this)),
      context_menu_(nullptr),
      root_(nullptr),
      library_backend_(nullptr),
      library_model_(nullptr),
      library_filter_(nullptr),
      library_sort_model_(new QSortFilterProxyModel(this)),
      load_database_task_id_(0),
      membership_(Membership_None),
      format_(Format_Ogg),
      total_song_count_(0),
      network_(new NetworkAccessManager(this)) {
  // Create the library backend in the database thread
  library_backend_ = new LibraryBackend;
  library_backend_->moveToThread(app_->database()->thread());
  library_backend_->Init(app_->database(), kSongsTable, QString::null,
                         QString::null, kFtsTable);
  library_model_ = new LibraryModel(library_backend_, app_, this);

  connect(library_backend_, SIGNAL(TotalSongCountUpdated(int)),
          SLOT(UpdateTotalSongCount(int)));

  library_sort_model_->setSourceModel(library_model_);
  library_sort_model_->setSortRole(LibraryModel::Role_SortText);
  library_sort_model_->setDynamicSortFilter(true);
  library_sort_model_->setSortLocaleAware(true);
  library_sort_model_->sort(0);

  app_->player()->RegisterUrlHandler(url_handler_);
  app_->global_search()->AddProvider(new LibrarySearchProvider(
      library_backend_, tr("Magnatune"), "magnatune",
      IconLoader::Load("magnatune", IconLoader::provider), 
      true, app_, this));
}

MagnatuneService::~MagnatuneService() { delete context_menu_; }

void MagnatuneService::ReloadSettings() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  membership_ = MembershipType(s.value("membership", Membership_None).toInt());
  username_ = s.value("username").toString();
  password_ = s.value("password").toString();
  format_ = PreferredFormat(s.value("format", Format_Ogg).toInt());
}

QStandardItem* MagnatuneService::CreateRootItem() {
  root_ = new QStandardItem(IconLoader::Load("magnatune", IconLoader::provider), 
                            kServiceName);
  root_->setData(true, InternetModel::Role_CanLazyLoad);
  return root_;
}

void MagnatuneService::LazyPopulate(QStandardItem* item) {
  switch (item->data(InternetModel::Role_Type).toInt()) {
    case InternetModel::Type_Service:
      library_model_->Init();
      if (total_song_count_ == 0 && !load_database_task_id_) {
        ReloadDatabase();
      }
      model()->merged_model()->AddSubModel(item->index(), library_sort_model_);
      break;

    default:
      break;
  }
}

void MagnatuneService::UpdateTotalSongCount(int count) {
  total_song_count_ = count;
}

void MagnatuneService::ReloadDatabase() {
  QNetworkRequest request = QNetworkRequest(QUrl(kDatabaseUrl));
  request.setAttribute(QNetworkRequest::CacheLoadControlAttribute,
                       QNetworkRequest::AlwaysNetwork);

  QNetworkReply* reply = network_->get(request);
  connect(reply, SIGNAL(finished()), SLOT(ReloadDatabaseFinished()));

  if (!load_database_task_id_)
    load_database_task_id_ =
        app_->task_manager()->StartTask(tr("Downloading Magnatune catalogue"));
}

void MagnatuneService::ReloadDatabaseFinished() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());

  app_->task_manager()->SetTaskFinished(load_database_task_id_);
  load_database_task_id_ = 0;

  if (reply->error() != QNetworkReply::NoError) {
    // TODO(David Sansome): Error handling
    qLog(Error) << reply->errorString();
    return;
  }

  if (root_->hasChildren()) root_->removeRows(0, root_->rowCount());

  // The XML file is compressed
  QtIOCompressor gzip(reply);
  gzip.setStreamFormat(QtIOCompressor::GzipFormat);
  if (!gzip.open(QIODevice::ReadOnly)) {
    qLog(Warning) << "Error opening gzip stream";
    return;
  }

  // Remove all existing songs in the database
  library_backend_->DeleteAll();

  // Parse the XML we got from Magnatune
  QXmlStreamReader reader(&gzip);
  SongList songs;
  while (!reader.atEnd()) {
    reader.readNext();

    if (reader.tokenType() == QXmlStreamReader::StartElement &&
        reader.name() == "Track") {
      songs << ReadTrack(reader);
    }
  }

  // Add the songs to the database
  library_backend_->AddOrUpdateSongs(songs);
  library_model_->Reset();
}

Song MagnatuneService::ReadTrack(QXmlStreamReader& reader) {
  Song song;

  while (!reader.atEnd()) {
    reader.readNext();

    if (reader.tokenType() == QXmlStreamReader::EndElement) break;

    if (reader.tokenType() == QXmlStreamReader::StartElement) {
      QStringRef name = reader.name();
      QString value = ReadElementText(reader);

      if (name == "artist") song.set_artist(value);
      if (name == "albumname") song.set_album(value);
      if (name == "trackname") song.set_title(value);
      if (name == "tracknum") song.set_track(value.toInt());
      if (name == "year") song.set_year(value.toInt());
      if (name == "magnatunegenres") song.set_genre(value.section(',', 0, 0));
      if (name == "seconds")
        song.set_length_nanosec(value.toInt() * kNsecPerSec);
      if (name == "cover_small") song.set_art_automatic(value);
      if (name == "albumsku") song.set_comment(value);
      if (name == "url") {
        QUrl url;
        // Magnatune's URLs are already encoded
        url.setEncodedUrl(value.toLocal8Bit());
        url.setScheme("magnatune");
        song.set_url(url);
      }
    }
  }

  song.set_valid(true);
  song.set_filetype(Song::Type_Stream);

  // We need to set these to satisfy the database constraints
  song.set_directory_id(0);
  song.set_mtime(0);
  song.set_ctime(0);
  song.set_filesize(0);

  return song;
}

// TODO(David Sansome): Replace with readElementText(SkipChildElements) in Qt 4.6
QString MagnatuneService::ReadElementText(QXmlStreamReader& reader) {
  int level = 1;
  QString ret;
  while (!reader.atEnd()) {
    switch (reader.readNext()) {
      case QXmlStreamReader::StartElement:
        level++;
        break;
      case QXmlStreamReader::EndElement:
        level--;
        break;
      case QXmlStreamReader::Characters:
        ret += reader.text().toString().trimmed();
        break;
      default:
        break;
    }

    if (level == 0) break;
  }
  return ret;
}

void MagnatuneService::EnsureMenuCreated() {
  if (context_menu_) return;

  context_menu_ = new QMenu;

  context_menu_->addActions(GetPlaylistActions());
  download_ = context_menu_->addAction(IconLoader::Load("download", 
                                       IconLoader::base),
                                       tr("Download this album"), this,
                                       SLOT(Download()));
  context_menu_->addSeparator();
  context_menu_->addAction(IconLoader::Load("download", IconLoader::base),
                           tr("Open %1 in browser").arg("magnatune.com"), this,
                           SLOT(Homepage()));
  context_menu_->addAction(IconLoader::Load("view-refresh", IconLoader::base),
                           tr("Refresh catalogue"), this,
                           SLOT(ReloadDatabase()));
  QAction* config_action = context_menu_->addAction(
      IconLoader::Load("configure", IconLoader::base), tr("Configure Magnatune..."), 
      this, SLOT(ShowConfig()));

  library_filter_ = new LibraryFilterWidget(0);
  library_filter_->SetSettingsGroup(kSettingsGroup);
  library_filter_->SetLibraryModel(library_model_);
  library_filter_->SetFilterHint(tr("Search Magnatune"));
  library_filter_->SetAgeFilterEnabled(false);
  library_filter_->AddMenuAction(config_action);

  context_menu_->addSeparator();
  context_menu_->addMenu(library_filter_->menu());
}

void MagnatuneService::ShowContextMenu(const QPoint& global_pos) {
  EnsureMenuCreated();

  const bool is_valid = model()->current_index().model() == library_sort_model_;

  GetAppendToPlaylistAction()->setEnabled(is_valid);
  GetReplacePlaylistAction()->setEnabled(is_valid);
  GetOpenInNewPlaylistAction()->setEnabled(is_valid);
  download_->setEnabled(is_valid && membership_ == Membership_Download);
  context_menu_->popup(global_pos);
}

void MagnatuneService::Homepage() {
  QDesktopServices::openUrl(QUrl(kHomepage));
}

QUrl MagnatuneService::ModifyUrl(const QUrl& url) const {
  QUrl ret(url);
  ret.setScheme("http");

  switch (membership_) {
    case Membership_None:
      return ret;  // Use the URL as-is

    // Otherwise add the hostname
    case Membership_Streaming:
      ret.setHost(kStreamingHostname);
      break;
    case Membership_Download:
      ret.setHost(kDownloadHostname);
      break;
  }

  // Add the credentials
  ret.setUserName(username_);
  ret.setPassword(password_);

  // And remove the commercial
  QString path = ret.path();
  path.insert(path.lastIndexOf('.'), "_nospeech");
  ret.setPath(path);

  return ret;
}

void MagnatuneService::ShowConfig() {
  app_->OpenSettingsDialogAtPage(SettingsDialog::Page_Magnatune);
}

void MagnatuneService::Download() {
  QModelIndex index =
      library_sort_model_->mapToSource(model()->current_index());
  SongList songs = library_model_->GetChildSongs(index);

  MagnatuneDownloadDialog* dialog = new MagnatuneDownloadDialog(this, 0);
  dialog->setAttribute(Qt::WA_DeleteOnClose);
  dialog->Show(songs);

  connect(dialog, SIGNAL(Finished(QStringList)),
          SIGNAL(DownloadFinished(QStringList)));
}

QWidget* MagnatuneService::HeaderWidget() const {
  const_cast<MagnatuneService*>(this)->EnsureMenuCreated();
  return library_filter_;
}
