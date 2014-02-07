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

class LibraryBackendInterface;

class LibrarySearchProvider : public BlockingSearchProvider {
 public:
  LibrarySearchProvider(LibraryBackendInterface* backend, const QString& name,
                        const QString& id, const QIcon& icon,
                        bool enabled_by_default, Application* app,
                        QObject* parent = 0);

  ResultList Search(int id, const QString& query);
  MimeData* LoadTracks(const ResultList& results);
  QStringList GetSuggestions(int count);

 private:
  LibraryBackendInterface* backend_;
};

#endif  // LIBRARYSEARCHPROVIDER_H
