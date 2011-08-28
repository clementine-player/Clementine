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

#ifndef LIBRARYSEARCHPROVIDER_H
#define LIBRARYSEARCHPROVIDER_H

#include "searchprovider.h"
#include "core/backgroundthread.h"

class AlbumCoverLoader;
class LibraryBackendInterface;


class LibrarySearchProvider : public BlockingSearchProvider {
  Q_OBJECT

public:
  LibrarySearchProvider(LibraryBackendInterface* backend, const QString& name,
                        const QIcon& icon, QObject* parent = 0);

  void LoadArtAsync(int id, const Result& result);
  void LoadTracksAsync(int id, const Result& result);

protected:
  ResultList Search(int id, const QString& query);

private slots:
  void AlbumArtLoaded(quint64 id, const QImage& image);

private:
  LibraryBackendInterface* backend_;

  BackgroundThread<AlbumCoverLoader>* cover_loader_;
  QMap<quint64, int> cover_loader_tasks_;
};

#endif // LIBRARYSEARCHPROVIDER_H
