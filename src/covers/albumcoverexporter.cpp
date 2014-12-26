/* This file is part of Clementine.
   Copyright 2013, Andreas <asfa194@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>
   Copyright 2014, John Maguire <john.maguire@gmail.com>

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

#include "albumcoverexporter.h"

#include <QFile>
#include <QThreadPool>

#include "coverexportrunnable.h"
#include "core/song.h"

const int AlbumCoverExporter::kMaxConcurrentRequests = 3;

AlbumCoverExporter::AlbumCoverExporter(QObject* parent)
    : QObject(parent),
      thread_pool_(new QThreadPool(this)),
      exported_(0),
      skipped_(0),
      all_(0) {
  thread_pool_->setMaxThreadCount(kMaxConcurrentRequests);
}

void AlbumCoverExporter::SetDialogResult(
    const AlbumCoverExport::DialogResult& dialog_result) {
  dialog_result_ = dialog_result;
}

void AlbumCoverExporter::AddExportRequest(Song song) {
  requests_.append(new CoverExportRunnable(dialog_result_, song));
  all_ = requests_.count();
}

void AlbumCoverExporter::Cancel() { requests_.clear(); }

void AlbumCoverExporter::StartExporting() {
  exported_ = 0;
  skipped_ = 0;
  AddJobsToPool();
}

void AlbumCoverExporter::AddJobsToPool() {
  while (!requests_.isEmpty() &&
         thread_pool_->activeThreadCount() < thread_pool_->maxThreadCount()) {
    CoverExportRunnable* runnable = requests_.dequeue();

    connect(runnable, SIGNAL(CoverExported()), SLOT(CoverExported()));
    connect(runnable, SIGNAL(CoverSkipped()), SLOT(CoverSkipped()));

    thread_pool_->start(runnable);
  }
}

void AlbumCoverExporter::CoverExported() {
  exported_++;
  emit AlbumCoversExportUpdate(exported_, skipped_, all_);
  AddJobsToPool();
}

void AlbumCoverExporter::CoverSkipped() {
  skipped_++;
  emit AlbumCoversExportUpdate(exported_, skipped_, all_);
  AddJobsToPool();
}
