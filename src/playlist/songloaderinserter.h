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

#ifndef SONGLOADERINSERTER_H
#define SONGLOADERINSERTER_H

#include <QObject>
#include <QSet>
#include <QUrl>

#include "core/song.h"

class LibraryBackendInterface;
class Player;
class Playlist;
class SongLoader;
class TaskManager;

class QModelIndex;

class SongLoaderInserter : public QObject {
  Q_OBJECT
public:
  SongLoaderInserter(TaskManager* task_manager,
                     LibraryBackendInterface* library,
                     const Player* player);
  ~SongLoaderInserter();

  void Load(Playlist* destination, int row, bool play_now, bool enqueue,
            const QList<QUrl>& urls);
  void LoadAudioCD(Playlist* destination, int row, bool play_now, bool enqueue);

signals:
  void Error(const QString& message);
  void EffectiveLoadFinished(const SongList& songs);

private slots:
  void PendingLoadFinished(bool success);
  void DestinationDestroyed();
  void AudioCDTagsLoaded(bool success);

private:
  void PartiallyFinished();
  void EffectiveLoad();
  void Finished();

private:
  TaskManager* task_manager_;

  Playlist* destination_;
  int row_;
  bool play_now_;
  bool enqueue_;

  SongList songs_;

  QSet<SongLoader*> pending_;
  QSet<SongLoader*> pending_async_;
  int async_load_id_;
  int async_progress_;
  LibraryBackendInterface* library_;
  const Player* player_;
};

#endif // SONGLOADERINSERTER_H
