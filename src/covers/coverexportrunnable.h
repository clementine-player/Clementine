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

#ifndef COVEREXPORTRUNNABLE_H
#define COVEREXPORTRUNNABLE_H

#include "core/song.h"
#include "ui/albumcoverexport.h"

#include <QObject>
#include <QRunnable>

class AlbumCoverExporter;

class CoverExportRunnable : public QObject, public QRunnable {
  Q_OBJECT

 public:
  CoverExportRunnable(const AlbumCoverExport::DialogResult& dialog_result,
                      const Song& song);
  virtual ~CoverExportRunnable() {}

  void run();

signals:
  void CoverExported();
  void CoverSkipped();

 private:
  void EmitCoverExported();
  void EmitCoverSkipped();

  void ProcessAndExportCover();
  void ExportCover();
  QString GetCoverPath();

  AlbumCoverExport::DialogResult dialog_result_;
  Song song_;
  AlbumCoverExporter* album_cover_exporter_;
};

#endif  // COVEREXPORTRUNNABLE_H
