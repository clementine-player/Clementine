/* This file is part of Clementine.
   Copyright 2011, David Sansome <me@davidsansome.com>

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

#ifndef TAGREADERCLIENT_H
#define TAGREADERCLIENT_H

#include "song.h"
#include "tagreadermessages.pb.h"
#include "core/messagehandler.h"
#include "core/workerpool.h"

#include <QStringList>

class QLocalServer;
class QProcess;

class TagReaderClient : public QObject {
  Q_OBJECT

 public:
  TagReaderClient(QObject* parent = 0);

  typedef AbstractMessageHandler<pb::tagreader::Message> HandlerType;
  typedef HandlerType::ReplyType ReplyType;

  static const char* kWorkerExecutableName;

  void Start();

  ReplyType* ReadFile(const QString& filename);
  ReplyType* SaveFile(const QString& filename, const Song& metadata);
  ReplyType* UpdateSongStatistics(const Song& metadata);
  ReplyType* UpdateSongRating(const Song& metadata);
  ReplyType* IsMediaFile(const QString& filename);
  ReplyType* LoadEmbeddedArt(const QString& filename);
  ReplyType* ReadCloudFile(const QUrl& download_url, const QString& title,
                           int size, const QString& mime_type,
                           const QString& authorisation_header);

  // Convenience functions that call the above functions and wait for a
  // response.  These block the calling thread with a semaphore, and must NOT
  // be called from the TagReaderClient's thread.
  void ReadFileBlocking(const QString& filename, Song* song);
  bool SaveFileBlocking(const QString& filename, const Song& metadata);
  bool UpdateSongStatisticsBlocking(const Song& metadata);
  bool UpdateSongRatingBlocking(const Song& metadata);
  bool IsMediaFileBlocking(const QString& filename);
  QImage LoadEmbeddedArtBlocking(const QString& filename);

  // TODO: Make this not a singleton
  static TagReaderClient* Instance() { return sInstance; }

 public slots:
  void UpdateSongsStatistics(const SongList& songs);
  void UpdateSongsRating(const SongList& songs);

 private slots:
  void WorkerFailedToStart();

 private:
  static TagReaderClient* sInstance;

  WorkerPool<HandlerType>* worker_pool_;
  QList<pb::tagreader::Message> message_queue_;
};

typedef TagReaderClient::ReplyType TagReaderReply;

#endif  // TAGREADERCLIENT_H
