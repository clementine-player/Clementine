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

#ifndef TAGFETCHER_H
#define TAGFETCHER_H

#include "config.h"

#include "core/network.h"
#include "core/song.h"

#include <QtDebug>
#include <QMap>
#include <QXmlStreamReader>

#ifdef HAVE_LIBTUNEPIMP
  #include <tunepimp-0.5/tp_c.h>
#endif

class TagFetcherItem;

class TagFetcher : public QObject {
  Q_OBJECT
#ifdef HAVE_LIBTUNEPIMP
 public:
  TagFetcher(QObject *parent = 0);
  ~TagFetcher();

  void FetchFromFile(const QString& path);
  void CallReturned(int fileId, TPFileStatus status);

 signals:
  void FetchFinished(const QString& filename, const SongList& songs_guessed); 
 
 private slots:
  void FetchFinishedSlot(int id);

 private:
  // Callback that will be called by libtunepimp
  static void NotifyCallback(tunepimp_t /*pimp*/, void */*data*/, TPCallbackEnum type, int fileId, TPFileStatus status);
  QMutex mutex_active_fetchers_;

  NetworkAccessManager *network_;
  tunepimp_t  pimp_;
  QMap<int, TagFetcherItem*> active_fetchers_;
#endif // HAVE_LIBTUNEPIMP 
};

class TagFetcherItem : public QObject {
  Q_OBJECT
#ifdef HAVE_LIBTUNEPIMP
 public:
  TagFetcherItem(const QString& filename, int filedId, tunepimp_t pimp, NetworkAccessManager *network);

  void setFileId(int _fileId) { fileId_ = _fileId; };
  int getFileId() { return fileId_; };
  void setFilename(const QString& _filename) { filename_ = _filename; };
  QString getFilename() { return filename_; }
  SongList getSongsFetched() { return songs_fetched_; }
  
  void Unrecognized();
  void PuidGenerated();
  
 signals:
  void PuidGeneratedSignal();
  void FetchFinishedSignal(int id);

 private slots:
  void PuidGeneratedSlot();
  void DownloadInfoFinished();

 private:
  SongList ReadTrack(QXmlStreamReader *reader);

 private:
  QString filename_;
  int     fileId_; // tunepimp generated id
  tunepimp_t pimp_;
  NetworkAccessManager *network_;
  char puid_[255];
  SongList  songs_fetched_;
#endif // HAVE_LIBTUNEPIMP
};

#endif // TAGFETCHER_H
