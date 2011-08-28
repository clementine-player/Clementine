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

#ifndef GLOBALSEARCH_H
#define GLOBALSEARCH_H

#include <QObject>

#include "searchprovider.h"


class GlobalSearch : public QObject {
  Q_OBJECT

public:
  GlobalSearch(QObject* parent = 0);

  void AddProvider(SearchProvider* provider);

  int SearchAsync(const QString& query);
  int LoadArtAsync(const SearchProvider::Result& result);

signals:
  void ResultsAvailable(int id, const SearchProvider::ResultList& results);
  void ProviderSearchFinished(int id, const SearchProvider* provider);
  void SearchFinished(int id);

  void ArtLoaded(int id, const QImage& image);

  void ProviderDestroyed(SearchProvider* provider);

private slots:
  void ResultsAvailableSlot(int id, const SearchProvider::ResultList& results);
  void SearchFinishedSlot(int id);

  void ProviderDestroyedSlot(QObject* object);

private:
  QList<SearchProvider*> providers_;

  int next_id_;
  QMap<int, int> pending_search_providers_;
};

#endif // GLOBALSEARCH_H
