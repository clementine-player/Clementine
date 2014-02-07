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

#ifndef ALBUMCOVEREXPORTER_H
#define ALBUMCOVEREXPORTER_H

#include "coverexportrunnable.h"
#include "core/song.h"
#include "ui/albumcoverexport.h"

#include <QObject>
#include <QQueue>
#include <QTimer>

class QThreadPool;

class AlbumCoverExporter : public QObject {
  Q_OBJECT

 public:
  AlbumCoverExporter(QObject* parent = 0);
  virtual ~AlbumCoverExporter() {}

  static const int kMaxConcurrentRequests;

  void SetDialogResult(const AlbumCoverExport::DialogResult& dialog_result);
  void AddExportRequest(Song song);
  void StartExporting();
  void Cancel();

  int request_count() { return requests_.size(); }

signals:
  void AlbumCoversExportUpdate(int exported, int skipped, int all);

 private slots:
  void CoverExported();
  void CoverSkipped();

 private:
  void AddJobsToPool();
  AlbumCoverExport::DialogResult dialog_result_;

  QQueue<CoverExportRunnable*> requests_;
  QThreadPool* thread_pool_;

  int exported_;
  int skipped_;
  int all_;
};

#endif  // ALBUMCOVEREXPORTER_H
