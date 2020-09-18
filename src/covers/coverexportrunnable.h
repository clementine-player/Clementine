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

#ifndef COVERS_COVEREXPORTRUNNABLE_H_
#define COVERS_COVEREXPORTRUNNABLE_H_

#include <QObject>
#include <QRunnable>

#include "core/song.h"
#include "ui/albumcoverexport.h"

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

#endif  // COVERS_COVEREXPORTRUNNABLE_H_
