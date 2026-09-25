/* This file is part of Clementine.
   Copyright 2026, John Maguire <john.maguire@gmail.com>

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

#ifndef INTERNET_PLEX_PLEXLIBRARYSCANNER_H_
#define INTERNET_PLEX_PLEXLIBRARYSCANNER_H_

#include <QMap>
#include <QObject>
#include <QStringList>

#include "core/song.h"

class QNetworkReply;
class PlexService;

// Fetches every track in a set of music sections, one page at a time.
class PlexLibraryScanner : public QObject {
  Q_OBJECT

 public:
  explicit PlexLibraryScanner(PlexService* service, QObject* parent = nullptr);

  static const int kPageSize;

  bool is_scanning() const { return scanning_; }

  void Scan(const QStringList& section_keys);

 signals:
  void Progress(int done, int total);
  // songs maps each section key to all of its tracks.  If ok is false the
  // scan was abandoned part way through and songs must not be used to delete
  // anything.
  void ScanFinished(const QMap<QString, SongList>& songs, bool ok);

 private slots:
  void PageFinished(QNetworkReply* reply, int offset);

 private:
  void RequestPage(int offset);
  void NextSection();
  void Finish(bool ok);

  PlexService* service_;
  bool scanning_;
  QStringList pending_sections_;
  QString current_section_;
  int done_;
  int total_;
  QMap<QString, SongList> songs_;
};

#endif  // INTERNET_PLEX_PLEXLIBRARYSCANNER_H_
