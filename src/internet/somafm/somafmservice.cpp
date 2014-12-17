/* This file is part of Clementine.
   Copyright 2010-2013, David Sansome <me@davidsansome.com>
   Copyright 2011, Tyler Rhodes <tyler.s.rhodes@gmail.com>
   Copyright 2011, Paweł Bara <keirangtp@gmail.com>
   Copyright 2012, 2014, John Maguire <john.maguire@gmail.com>
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

#include "somafmservice.h"
#include "somafmurlhandler.h"
#include "internetmodel.h"
#include "core/application.h"
#include "core/closure.h"
#include "core/logging.h"
#include "core/network.h"
#include "core/player.h"
#include "core/taskmanager.h"
#include "core/utilities.h"
#include "globalsearch/globalsearch.h"
#include "globalsearch/somafmsearchprovider.h"
#include "ui/iconloader.h"

#include <QCoreApplication>
#include <QDesktopServices>
#include <QMenu>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QXmlStreamReader>
#include <QtDebug>

const int SomaFMServiceBase::kStreamsCacheDurationSecs =
    60 * 60 * 24 * 28;  // 4 weeks

bool operator<(const SomaFMServiceBase::Stream& a,
               const SomaFMServiceBase::Stream& b) {
  return a.title_.compare(b.title_, Qt::CaseInsensitive) < 0;
}

SomaFMServiceBase::SomaFMServiceBase(Application* app, InternetModel* parent,
                                     const QString& name,
                                     const QUrl& channel_list_url,
                                     const QUrl& homepage_url,
                                     const QUrl& donate_page_url,
                                     const QIcon& icon)
    : InternetService(name, app, parent, parent),
      url_scheme_(name.toLower().remove(' ')),
      url_handler_(new SomaFMUrlHandler(app, this, this)),
      root_(nullptr),
      context_menu_(nullptr),
      network_(new NetworkAccessManager(this)),
      streams_(name, "streams", kStreamsCacheDurationSecs),
      name_(name),
      channel_list_url_(channel_list_url),
      homepage_url_(homepage_url),
      donate_page_url_(donate_page_url),
      icon_(icon) {
  ReloadSettings();

  app_->player()->RegisterUrlHandler(url_handler_);
  app_->global_search()->AddProvider(
      new SomaFMSearchProvider(this, app_, this));
}

SomaFMServiceBase::~SomaFMServiceBase() { delete context_menu_; }

QStandardItem* SomaFMServiceBase::CreateRootItem() {
  root_ = new QStandardItem(icon_, name_);
  root_->setData(true, InternetModel::Role_CanLazyLoad);
  return root_;
}

void SomaFMServiceBase::LazyPopulate(QStandardItem* item) {
  switch (item->data(InternetModel::Role_Type).toInt()) {
    case InternetModel::Type_Service:
      RefreshStreams();
      break;

    default:
      break;
  }
}

void SomaFMServiceBase::ShowContextMenu(const QPoint& global_pos) {
  if (!context_menu_) {
    context_menu_ = new QMenu;
    context_menu_->addActions(GetPlaylistActions());
    context_menu_->addAction(IconLoader::Load("download"),
                             tr("Open %1 in browser").arg(homepage_url_.host()),
                             this, SLOT(Homepage()));

    if (!donate_page_url_.isEmpty()) {
      context_menu_->addAction(IconLoader::Load("download"), tr("Donate"), this,
                               SLOT(Donate()));
    }

    context_menu_->addAction(IconLoader::Load("view-refresh"),
                             tr("Refresh channels"), this,
                             SLOT(ForceRefreshStreams()));
  }

  context_menu_->popup(global_pos);
}

void SomaFMServiceBase::ForceRefreshStreams() {
  QNetworkReply* reply = network_->get(QNetworkRequest(channel_list_url_));
  int task_id = app_->task_manager()->StartTask(tr("Getting channels"));

  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(RefreshStreamsFinished(QNetworkReply*, int)), reply, task_id);
}

void SomaFMServiceBase::RefreshStreamsFinished(QNetworkReply* reply,
                                               int task_id) {
  app_->task_manager()->SetTaskFinished(task_id);
  reply->deleteLater();

  if (reply->error() != QNetworkReply::NoError) {
    // TODO(David Sansome): Error handling
    qLog(Error) << reply->errorString();
    return;
  }

  StreamList list;

  QXmlStreamReader reader(reply);
  while (!reader.atEnd()) {
    reader.readNext();

    if (reader.tokenType() == QXmlStreamReader::StartElement &&
        reader.name() == "channel") {
      ReadChannel(reader, &list);
    }
  }

  streams_.Update(list);
  streams_.Sort();

  // Only update the item's children if it's already been populated
  if (!root_->data(InternetModel::Role_CanLazyLoad).toBool()) PopulateStreams();

  emit StreamsChanged();
}

void SomaFMServiceBase::ReadChannel(QXmlStreamReader& reader, StreamList* ret) {
  Stream stream;

  while (!reader.atEnd()) {
    switch (reader.readNext()) {
      case QXmlStreamReader::EndElement:
        if (!stream.url_.isEmpty()) {
          ret->append(stream);
        }
        return;

      case QXmlStreamReader::StartElement:
        if (reader.name() == "title") {
          stream.title_ = reader.readElementText();
        } else if (reader.name() == "dj") {
          stream.dj_ = reader.readElementText();
        } else if (reader.name() == "fastpls" &&
                   reader.attributes().value("format") == "mp3") {
          QUrl url(reader.readElementText());
          url.setScheme(url_handler_->scheme());

          stream.url_ = url;
        } else {
          Utilities::ConsumeCurrentElement(&reader);
        }
        break;

      default:
        break;
    }
  }
}

Song SomaFMServiceBase::Stream::ToSong(const QString& prefix) const {
  QString song_title = title_.trimmed();
  if (!song_title.startsWith(prefix)) {
    song_title = prefix + " " + song_title;
  }

  Song ret;
  ret.set_valid(true);
  ret.set_title(song_title);
  ret.set_artist(dj_);
  ret.set_url(url_);
  return ret;
}

void SomaFMServiceBase::Homepage() { QDesktopServices::openUrl(homepage_url_); }

void SomaFMServiceBase::Donate() {
  QDesktopServices::openUrl(donate_page_url_);
}

PlaylistItem::Options SomaFMServiceBase::playlistitem_options() const {
  return PlaylistItem::PauseDisabled;
}

SomaFMServiceBase::StreamList SomaFMServiceBase::Streams() {
  if (IsStreamListStale()) {
    metaObject()->invokeMethod(this, "ForceRefreshStreams",
                               Qt::QueuedConnection);
  }
  return streams_;
}

void SomaFMServiceBase::RefreshStreams() {
  if (IsStreamListStale()) {
    ForceRefreshStreams();
    return;
  }
  PopulateStreams();
}

void SomaFMServiceBase::PopulateStreams() {
  if (root_->hasChildren()) root_->removeRows(0, root_->rowCount());

  for (const Stream& stream : streams_) {
    QStandardItem* item =
        new QStandardItem(QIcon(":last.fm/icon_radio.png"), QString());
    item->setText(stream.title_);
    item->setData(QVariant::fromValue(stream.ToSong(name_)),
                  InternetModel::Role_SongMetadata);
    item->setData(InternetModel::PlayBehaviour_SingleItem,
                  InternetModel::Role_PlayBehaviour);

    root_->appendRow(item);
  }
}

QDataStream& operator<<(QDataStream& out,
                        const SomaFMServiceBase::Stream& stream) {
  out << stream.title_ << stream.dj_ << stream.url_;
  return out;
}

QDataStream& operator>>(QDataStream& in, SomaFMServiceBase::Stream& stream) {
  in >> stream.title_ >> stream.dj_ >> stream.url_;
  return in;
}

void SomaFMServiceBase::ReloadSettings() {
  streams_.Load();
  streams_.Sort();
}

SomaFMService::SomaFMService(Application* app, InternetModel* parent)
    : SomaFMServiceBase(
          app, parent, "SomaFM", QUrl("http://somafm.com/channels.xml"),
          QUrl("http://somafm.com"), QUrl(), QIcon(":providers/somafm.png")) {}

RadioGFMService::RadioGFMService(Application* app, InternetModel* parent)
    : SomaFMServiceBase(app, parent, "Radio GFM",
                        QUrl("http://streams.radio-gfm.net/channels.xml"),
                        QUrl("http://www.radio-gfm.net"),
                        QUrl("http://www.radio-gfm.net/spenden"),
                        QIcon(":providers/radiogfm.png")) {}
