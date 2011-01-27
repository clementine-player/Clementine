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

#include "tagfetcher.h"

#include <QtDebug>
#include <QCoreApplication>
#include <QFile>
#include <QMutex>
#include <QNetworkReply>
#include <QStringList>

#include "version.h"

#ifdef HAVE_LIBTUNEPIMP

const char* kMusicBrainzLookupUrl = "http://musicbrainz.org/ws/1/track/?type=xml&puid=%1";

TagFetcher::TagFetcher(QObject *parent)
    : QObject(parent),
      network_(new NetworkAccessManager(this)),
      pimp_(NULL)
{
  QString plugin_path =
  #ifdef Q_OS_DARWIN
      QCoreApplication::applicationDirPath() + "/../PlugIns/tunepimp";
  #else
      "";
  #endif
  pimp_ = tp_NewWithArgs(
      QCoreApplication::applicationName().toAscii().constData(),
      CLEMENTINE_VERSION,
      TP_THREAD_ALL,
      plugin_path.toLocal8Bit().constData());

  tp_SetDebug(pimp_, true);
  tp_SetAutoSaveThreshold(pimp_, -1);
  tp_SetMoveFiles(pimp_, false);
  tp_SetRenameFiles(pimp_, false);
  tp_SetFileNameEncoding(pimp_, "UTF-8");
  tp_SetNotifyCallback(pimp_, NotifyCallback, this);
  tp_SetMusicDNSClientId(pimp_, "0c6019606b1d8a54d0985e448f3603ca");
}

TagFetcher::~TagFetcher() {
  delete network_;
  tp_Delete(pimp_);
}

void TagFetcher::FetchFromFile(const QString& path) {
  mutex_active_fetchers_.lock();
  int id = tp_AddFile(pimp_, QFile::encodeName(path), 0);
  TagFetcherItem* tagFetcherItem = new TagFetcherItem(path, id, pimp_, network_);
  active_fetchers_.insert(id, tagFetcherItem);
  mutex_active_fetchers_.unlock();
  connect(tagFetcherItem, SIGNAL(PuidGeneratedSignal()), tagFetcherItem, SLOT(PuidGeneratedSlot()), Qt::QueuedConnection);
  connect(tagFetcherItem, SIGNAL(FetchFinishedSignal(int)), SLOT(FetchFinishedSlot(int)));
}

void TagFetcher::NotifyCallback(tunepimp_t pimp, void *data, TPCallbackEnum type, int fileId, TPFileStatus status)
{
  if(type == tpFileChanged) {
    TagFetcher *tagFetcher = (TagFetcher*)data;
    tagFetcher->CallReturned(fileId, status);
  }
}

void TagFetcher::CallReturned(int fileId, TPFileStatus status)
{
  // Using a lock, to prevent getting NULL item, when callback is called but
  // item not yet inserted: this could happen because libtunepimp proceed in
  // a separate thread after tp_AddFile call
  mutex_active_fetchers_.lock();
  TagFetcherItem *tagFetcherItem = active_fetchers_.value(fileId);
  mutex_active_fetchers_.unlock();
  switch(status) {
    case eRecognized:
      // TODO
      break;
    case eUnrecognized:
      tagFetcherItem->Unrecognized();
      break;
    case ePUIDLookup:
    case ePUIDCollision:
    case eFileLookup:
      //TODO
      tagFetcherItem->PuidGenerated();
      break;
    case eUserSelection:
      // TODO
      break;
    case eError:
      //TODO
      break;
    default:
      break;
  }
}

void TagFetcher::FetchFinishedSlot(int id) {
  TagFetcherItem *tagFetcherItem = active_fetchers_.value(id);
  tp_Remove(pimp_, id);
  active_fetchers_.erase(active_fetchers_.find(id));
  emit FetchFinished(tagFetcherItem->getFilename(), tagFetcherItem->getSongsFetched());
  delete tagFetcherItem;
}

TagFetcherItem::TagFetcherItem(const QString& _filename, int _fileId, tunepimp_t _pimp, NetworkAccessManager *_network)
  : filename_(_filename),
    fileId_(_fileId),
    pimp_(_pimp),
    network_(_network),
    already_tried_to_recognize_(false)
{ }

void TagFetcherItem::Unrecognized() {
  if(already_tried_to_recognize_) {
    // We already tried to recognize this music and, apparently, nothing has been found: stopping here
    emit FetchFinishedSignal(fileId_);
  } else {
    already_tried_to_recognize_ = true;
    char trm[255];
    trm[0] = 0;
    track_t track = tp_GetTrack(pimp_, fileId_);
    tr_Lock(track);
    tr_GetPUID(track, trm, 255);
    if ( !trm[0] ) {
      tr_SetStatus(track, ePending);
      tp_Wake(pimp_, track);
    }
    tr_Unlock(track);
    tp_ReleaseTrack(pimp_, track);
  }
}

void TagFetcherItem::PuidGenerated() {
    // Get PUID
    track_t track = tp_GetTrack(pimp_, fileId_);
    tr_Lock(track);
    tr_GetPUID(track, puid_, 255);
    tr_Unlock(track);
    tp_ReleaseTrack(pimp_, track);

    // Now we have PUID, let's download song's info from musicbrainz's Web Service
    emit PuidGeneratedSignal();
}

void TagFetcherItem::PuidGeneratedSlot() {
  QString reqString(QString(kMusicBrainzLookupUrl).arg(QString(puid_)));
  QNetworkRequest req = QNetworkRequest(QUrl(reqString));
  req.setAttribute(QNetworkRequest::CacheLoadControlAttribute,
                   QNetworkRequest::AlwaysNetwork);
  QNetworkReply* reply = network_->get(req);
  connect(reply, SIGNAL(finished()), SLOT(DownloadInfoFinished()));
}

void TagFetcherItem::DownloadInfoFinished() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  Q_ASSERT(reply);

  QXmlStreamReader reader(reply);
  while(!reader.atEnd()) {
    reader.readNext();
    if(reader.tokenType() == QXmlStreamReader::StartElement &&
        reader.name() == "track") {
      songs_fetched_ << ReadTrack(&reader);
    }
  }
  emit FetchFinishedSignal(fileId_);
}

SongList TagFetcherItem::ReadTrack(QXmlStreamReader *reader) {
  QString currentArtist;
  QString currentTitle;
  SongList songs;
  while(!reader->atEnd()) {
    reader->readNext();
    if(reader->tokenType() == QXmlStreamReader::StartElement) {
      if(reader->name() == "title") { // track's title
        currentTitle = reader->readElementText();
      } else if(reader->name() == "artist") {
        reader->readNext();
        if(reader->name() == "name") {
          currentArtist = reader->readElementText();
        }
      } else if(reader->name() == "release-list") {
        reader->readNext();
        while(!reader->atEnd()) {
          if(reader->name() == "title") { // album (release) title
            QString albumTitle = reader->readElementText();
            Song newSongMatch;
            newSongMatch.Init(currentTitle, currentArtist, albumTitle, 0); // Using 0 as length. We could have used <duration> field but it is not usefull, so don't wasting our time and keeping 0
            reader->readNext();
            if(reader->name() == "track-list") {
              QXmlStreamAttributes attributes = reader->attributes();
              if(attributes.hasAttribute("offset")) {
                int track = attributes.value("offset").toString().toInt()+1;
                newSongMatch.set_track(track);
              }
              reader->readNext();
            }
            songs << newSongMatch;
          } else if(reader->tokenType() == QXmlStreamReader::EndElement &&
              reader->name() == "release-list") {
            break;
          } else {
            reader->readNext();
          }
        }
      }
    }
  }
  return songs;
}

#endif // HAVE_LIBTUNEPIMP
