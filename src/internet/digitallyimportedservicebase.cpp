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

#include "digitallyimportedclient.h"
#include "digitallyimportedservicebase.h"
#include "digitallyimportedurlhandler.h"
#include "internetmodel.h"
#include "core/logging.h"
#include "core/network.h"
#include "core/player.h"
#include "core/taskmanager.h"
#include "globalsearch/digitallyimportedsearchprovider.h"
#include "globalsearch/globalsearch.h"
#include "ui/iconloader.h"

#include <QDesktopServices>
#include <QMenu>
#include <QNetworkReply>
#include <QSettings>

const char* DigitallyImportedServiceBase::kSettingsGroup = "digitally_imported";
const int DigitallyImportedServiceBase::kStreamsCacheDurationSecs =
    60 * 60 * 24 * 14; // 2 weeks


DigitallyImportedServiceBase::DigitallyImportedServiceBase(
    const QString& name, InternetModel* model, QObject* parent)
  : InternetService(name, model, parent),
    network_(new NetworkAccessManager(this)),
    url_handler_(new DigitallyImportedUrlHandler(this)),
    basic_audio_type_(1),
    premium_audio_type_(2),
    task_id_(-1),
    root_(NULL),
    context_menu_(NULL),
    context_item_(NULL),
    api_client_(NULL)
{
  basic_playlists_
      << "http://listen.%1/public3/%2.pls"
      << "http://listen.%1/public1/%2.pls"
      << "http://listen.%1/public5/%2.asx";

  premium_playlists_
      << "http://listen.%1/premium_high/%2.pls?hash=%3"
      << "http://listen.%1/premium_medium/%2.pls?hash=%3"
      << "http://listen.%1/premium/%2.pls?hash=%3"
      << "http://listen.%1/premium_wma_low/%2.asx?hash=%3"
      << "http://listen.%1/premium_wma/%2.asx?hash=%3";
}

void DigitallyImportedServiceBase::Init(
    const QString& description, const QUrl& homepage_url,
    const QString& homepage_name, const QUrl& stream_list_url,
    const QString& url_scheme, const QString& icon_path,
    const QString& api_service_name) {
  service_description_ = description;
  homepage_url_ = homepage_url;
  homepage_name_ = homepage_name;
  stream_list_url_ = stream_list_url;
  url_scheme_ = url_scheme;
  icon_path_ = icon_path;
  api_service_name_ = api_service_name;
  icon_ = QIcon(icon_path_);

  model()->player()->RegisterUrlHandler(url_handler_);
  model()->global_search()->AddProvider(new DigitallyImportedSearchProvider(this, this));

  api_client_ = new DigitallyImportedClient(api_service_name, this);
}

DigitallyImportedServiceBase::~DigitallyImportedServiceBase() {
  delete context_menu_;
}

QStandardItem* DigitallyImportedServiceBase::CreateRootItem() {
  root_ = new QStandardItem(icon_, service_description_);
  root_->setData(true, InternetModel::Role_CanLazyLoad);
  return root_;
}

void DigitallyImportedServiceBase::LazyPopulate(QStandardItem* parent) {
  if (parent == root_) {
    RefreshStreams();
  }
}

void DigitallyImportedServiceBase::RefreshStreams() {
  if (IsStreamListStale()) {
    ForceRefreshStreams();
    return;
  }

  PopulateStreams();
}

void DigitallyImportedServiceBase::ForceRefreshStreams() {
  if (task_id_ != -1) {
    return;
  }

  qLog(Info) << "Getting stream list from" << stream_list_url_;

  // Get the list of streams
  QNetworkReply* reply = network_->get(QNetworkRequest(stream_list_url_));
  connect(reply, SIGNAL(finished()), SLOT(RefreshStreamsFinished()));

  // Start a task to tell the user we're busy
  task_id_ = model()->task_manager()->StartTask(tr("Getting streams"));
}

void DigitallyImportedServiceBase::RefreshStreamsFinished() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (!reply) {
    return;
  }

  model()->task_manager()->SetTaskFinished(task_id_);
  reply->deleteLater();

  const QString data = QString::fromUtf8(reply->readAll());

  // Poor man's JSON parser that's good enough for the stream lists and means
  // we don't have to pull in QJSON as a dependency.
  const QRegExp re("\\{"
                   "\"id\":(\\d+),"
                   "\"key\":\"([^\"]+)\","
                   "\"name\":\"([^\"]+)\","
                   "\"description\":\"([^\"]+)\"");

  saved_streams_.clear();

  int pos = 0;
  while (pos >= 0) {
    pos = re.indexIn(data, pos);
    if (pos == -1) {
      break;
    }
    pos += re.matchedLength();

    Stream stream;
    stream.id_          = re.cap(1).toInt();
    stream.key_         = re.cap(2).replace("\\/", "/");
    stream.name_        = re.cap(3).replace("\\/", "/");
    stream.description_ = re.cap(4).replace("\\/", "/");
    saved_streams_ << stream;
  }

  // Sort by name
  qSort(saved_streams_);

  SaveStreams(saved_streams_);
  PopulateStreams();

  emit StreamsChanged();
}

void DigitallyImportedServiceBase::PopulateStreams() {
  if (root_->hasChildren())
    root_->removeRows(0, root_->rowCount());

  // Add each stream to the model
  foreach (const Stream& stream, saved_streams_) {
    Song song;
    song.set_title(stream.name_);
    song.set_artist(service_description_);
    song.set_url(QUrl(url_scheme_ + "://" + stream.key_));

    QStandardItem* item = new QStandardItem(QIcon(":/last.fm/icon_radio.png"),
                                            stream.name_);
    item->setData(stream.description_, Qt::ToolTipRole);
    item->setData(InternetModel::PlayBehaviour_SingleItem, InternetModel::Role_PlayBehaviour);
    item->setData(QVariant::fromValue(song), InternetModel::Role_SongMetadata);
    root_->appendRow(item);
  }
}

void DigitallyImportedServiceBase::Homepage() {
  QDesktopServices::openUrl(homepage_url_);
}

void DigitallyImportedServiceBase::ReloadSettings() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  basic_audio_type_ = s.value("basic_audio_type", 1).toInt();
  premium_audio_type_ = s.value("premium_audio_type", 2).toInt();
  username_ = s.value("username").toString();
  listen_hash_ = s.value("listen_hash").toString();
  last_refreshed_streams_ = s.value("last_refreshed_" + url_scheme_).toDateTime();
  saved_streams_ = LoadStreams();
}

void DigitallyImportedServiceBase::ShowContextMenu(
    const QModelIndex& index, const QPoint& global_pos) {
  if (!context_menu_) {
    context_menu_ = new QMenu;
    context_menu_->addActions(GetPlaylistActions());
    context_menu_->addAction(IconLoader::Load("download"),
                             tr("Open %1 in browser").arg(homepage_name_),
                             this, SLOT(Homepage()));
    context_menu_->addAction(IconLoader::Load("view-refresh"),
                             tr("Refresh streams"),
                             this, SLOT(ForceRefreshStreams()));
    context_menu_->addSeparator();
    context_menu_->addAction(IconLoader::Load("configure"),
                             tr("Configure..."),
                             this, SLOT(ShowSettingsDialog()));
  }

  context_item_ = model()->itemFromIndex(index);
  context_menu_->popup(global_pos);
}

bool DigitallyImportedServiceBase::is_premium_account() const {
  return !listen_hash_.isEmpty();
}

void DigitallyImportedServiceBase::LoadPlaylistFinished() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (!reply) {
    return;
  }
  reply->deleteLater();

  if (reply->header(QNetworkRequest::ContentTypeHeader).toString() == "text/html") {
    url_handler_->CancelTask();

    emit StreamError(tr("Error loading di.fm playlist"));
  } else {
    url_handler_->LoadPlaylistFinished(reply);
  }
}

void DigitallyImportedServiceBase::ShowSettingsDialog() {
  emit OpenSettingsAtPage(SettingsDialog::Page_DigitallyImported);
}

DigitallyImportedServiceBase::StreamList DigitallyImportedServiceBase::LoadStreams() const {
  StreamList ret;

  QSettings s;
  s.beginGroup(kSettingsGroup);

  int count = s.beginReadArray(url_scheme_);
  for (int i=0 ; i<count ; ++i) {
    s.setArrayIndex(i);

    Stream stream;
    stream.id_ = s.value("id").toInt();
    stream.key_ = s.value("key").toString();
    stream.name_ = s.value("name").toString();
    stream.description_ = s.value("description").toString();
    ret << stream;
  }
  s.endArray();

  return ret;
}

void DigitallyImportedServiceBase::SaveStreams(const StreamList& streams) {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  s.beginWriteArray(url_scheme_, streams.count());
  for (int i=0 ; i<streams.count() ; ++i) {
    const Stream& stream = streams[i];
    s.setArrayIndex(i);
    s.setValue("id", stream.id_);
    s.setValue("key", stream.key_);
    s.setValue("name", stream.name_);
    s.setValue("description", stream.description_);
  }
  s.endArray();

  last_refreshed_streams_ = QDateTime::currentDateTime();
  s.setValue("last_refreshed_" + url_scheme_, last_refreshed_streams_);
}

bool DigitallyImportedServiceBase::IsStreamListStale() const {
  return last_refreshed_streams_.isNull() ||
         last_refreshed_streams_.secsTo(QDateTime::currentDateTime()) >
            kStreamsCacheDurationSecs;
}

DigitallyImportedServiceBase::StreamList DigitallyImportedServiceBase::Streams() {
  if (IsStreamListStale()) {
    metaObject()->invokeMethod(this, "ForceRefreshStreams", Qt::QueuedConnection);
  }

  return saved_streams_;
}

void DigitallyImportedServiceBase::LoadStation(const QString& key) {
  QUrl playlist_url;

  if (is_premium_account()) {
    playlist_url = QUrl(premium_playlists_[premium_audio_type_].arg(
                          homepage_name_, key, listen_hash_));
  } else {
    playlist_url = QUrl(basic_playlists_[basic_audio_type_].arg(
                          homepage_name_, key));
  }

  qLog(Debug) << "Getting playlist URL" << playlist_url;

  QNetworkReply* reply = network_->get(QNetworkRequest(playlist_url));
  connect(reply, SIGNAL(finished()), SLOT(LoadPlaylistFinished()));
}

QModelIndex DigitallyImportedServiceBase::GetCurrentIndex() {
  return context_item_->index();
}
