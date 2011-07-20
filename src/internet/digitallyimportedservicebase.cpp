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

#include "digitallyimportedservicebase.h"
#include "digitallyimportedurlhandler.h"
#include "internetmodel.h"
#include "core/logging.h"
#include "core/network.h"
#include "core/player.h"
#include "core/taskmanager.h"
#include "ui/iconloader.h"

#include <QDesktopServices>
#include <QMenu>
#include <QNetworkReply>
#include <QSettings>

const char* DigitallyImportedServiceBase::kSettingsGroup = "digitally_imported";


DigitallyImportedServiceBase::DigitallyImportedServiceBase(
  const QString& name, const QString& description, const QUrl& homepage_url,
  const QString& homepage_name, const QUrl& stream_list_url,
  const QString& url_scheme, const QIcon& icon,
  InternetModel* model, QObject* parent)
  : InternetService(name, model, parent),
    network_(new NetworkAccessManager(this)),
    url_handler_(new DigitallyImportedUrlHandler(this)),
    audio_type_(0),
    task_id_(-1),
    homepage_url_(homepage_url),
    homepage_name_(homepage_name),
    stream_list_url_(stream_list_url),
    icon_(icon),
    service_description_(description),
    url_scheme_(url_scheme),
    root_(NULL),
    context_menu_(NULL),
    context_item_(NULL)
{
  model->player()->RegisterUrlHandler(url_handler_);
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

  QList<Stream> streams;

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
    streams << stream;
  }

  // Sort by name
  qSort(streams);

  // Add each stream to the model
  foreach (const Stream& stream, streams) {
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

  audio_type_ = s.value("audio_type", 0).toInt();
  username_ = s.value("username").toString();
  password_ = s.value("password").toString();
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
                             this, SLOT(RefreshStreams()));
    context_menu_->addSeparator();
    context_menu_->addAction(IconLoader::Load("configure"),
                             tr("Configure..."),
                             this, SLOT(ShowSettingsDialog()));
  }

  context_item_ = model()->itemFromIndex(index);
  context_menu_->popup(global_pos);
}

QModelIndex DigitallyImportedServiceBase::GetCurrentIndex() {
  return context_item_->index();
}

bool DigitallyImportedServiceBase::is_valid_stream_selected() const {
  return audio_type_ >= 0 && audio_type_ < playlists_.count();
}

bool DigitallyImportedServiceBase::is_premium_account() const {
  return !username_.isEmpty() && !password_.isEmpty();
}

bool DigitallyImportedServiceBase::is_premium_stream_selected() const {
  if (!is_valid_stream_selected()) {
    return false;
  }

  return playlists_[audio_type_].premium_;
}

void DigitallyImportedServiceBase::LoadPlaylistFinished() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (!reply) {
    return;
  }
  reply->deleteLater();

  if (reply->header(QNetworkRequest::ContentTypeHeader).toString() == "text/html") {
    url_handler_->CancelTask();

    if (is_premium_stream_selected()) {
      emit StreamError(tr("Invalid di.fm username or password"));
    } else {
      emit StreamError(tr("Error loading di.fm playlist"));
    }
  } else {
    url_handler_->LoadPlaylistFinished(reply);
  }
}

void DigitallyImportedServiceBase::ShowSettingsDialog() {
  emit OpenSettingsAtPage(SettingsDialog::Page_DigitallyImported);
}
