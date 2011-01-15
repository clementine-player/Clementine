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
#include <QFile>
#include <QStringList>
#include <QNetworkReply>

#ifdef HAVE_LIBTUNEPIMP

static void NotifyCallback(tunepimp_t /*pimp*/, void */*data*/, TPCallbackEnum type, int fileId, TPFileStatus status);

const char* kMusicBrainzLookupUrl = "http://musicbrainz.org/ws/1/track/?type=xml&puid=%1";

TagFetcher::TagFetcher(QObject *parent)
    : QObject(parent),
      network_(new NetworkAccessManager(this)),
      pimp_(NULL)
{
  //active_fetchers_ = new QMap<int, TagFetcherItem*>;
  pimp_ = tp_New("FETCHTAG", "0.1");
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
  int id = tp_AddFile(pimp_, QFile::encodeName(path), 0);
  TagFetcherItem* tagFetcherItem = new TagFetcherItem(path, id, pimp_, network_);
  connect(tagFetcherItem, SIGNAL(PuidGeneratedSignal()), tagFetcherItem, SLOT(PuidGeneratedSlot()), Qt::QueuedConnection);
  active_fetchers_.insert(id, tagFetcherItem);
  connect(tagFetcherItem, SIGNAL(FetchFinishedSignal(int)), SLOT(FetchFinishedSlot(int)));
}

static void NotifyCallback(tunepimp_t pimp, void *data, TPCallbackEnum type, int fileId, TPFileStatus status)
{
  TagFetcher *tagFetcher = (TagFetcher*)data;
  tagFetcher->CallReturned(fileId, status);
  if(type != tpFileChanged)
    return;
}

void TagFetcher::CallReturned(int fileId, TPFileStatus status)
{
  /*for(QMap<int, TagFetcherItem*>::Iterator it = active_fetchers_.begin();
      it != active_fetchers_.end(); ++it) {
    qDebug() <<  it.key();
    active_fetchers_.insert(it.key(), it.value());
  }*/
  TagFetcherItem *tagFetcherItem = active_fetchers_.value(fileId);
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

TagFetcherItem::TagFetcherItem() { }

TagFetcherItem::TagFetcherItem(const QString& _filename, int _fileId, tunepimp_t _pimp, NetworkAccessManager *_network)
  : filename_(_filename),
    fileId_(_fileId),
    pimp_(_pimp),
    network_(_network)
{ }

TagFetcherItem::~TagFetcherItem()
{ }

void TagFetcherItem::Unrecognized() {
  char trm[255];
  bool finish = false;
  trm[0] = 0;
  track_t track = tp_GetTrack(pimp_, fileId_);
  tr_Lock(track);
  tr_GetPUID(track, trm, 255);
  if ( !trm[0] ) {
    tr_SetStatus(track, ePending);
    tp_Wake(pimp_, track);
  }
  else {
    finish = true;
  }
  tr_Unlock(track);
  tp_ReleaseTrack(pimp_, track);
  if ( !finish )
    return;
  //TODO clear
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
        while(!reader->atEnd()) {
          reader->readNext();
          if(reader->name() == "title") { // album (release) title
            QString albumTitle = reader->readElementText();
            Song newSongMatch;
            newSongMatch.Init(currentTitle, currentArtist, albumTitle, 0); // Using 0 as length. We could have used <duration> field but it is not usefull, so don't wasting our time and keeping 0
            songs << newSongMatch;
          } else if(reader->tokenType() == QXmlStreamReader::EndElement &&
              reader->name() == "release-list") {
            break;
          }
        }
      }
    }
  }
  return songs;
}

#endif // HAVE_LIBTUNEPIMP
