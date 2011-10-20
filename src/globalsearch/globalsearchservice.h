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

#ifndef GLOBALSEARCHSERVICE_H
#define GLOBALSEARCHSERVICE_H

#include <QDBusArgument>
#include <QObject>
#include <QSet>

#include "common.h"
#include "searchprovider.h"

class GlobalSearch;

class QEventLoop;


class GlobalSearchService : public QObject {
  Q_OBJECT

public:
  GlobalSearchService(GlobalSearch* engine, QObject* parent = 0);

public slots:
  int StartSearch(const QString& query, bool prefetch_art);
  void CancelSearch(int id);

signals:
  void ResultsAvailable(int id, const GlobalSearchServiceResultList& results);
  void SearchFinished(int id);
  void ArtLoaded(int result_id, const QByteArray& image_data);

private slots:
  void ResultsAvailableSlot(int id, const SearchProvider::ResultList& results);
  void SearchFinishedSlot(int id);
  void ArtLoadedSlot(int id, const QPixmap& pixmap);

private:
  struct PendingSearch {
    bool prefetch_art_;
  };

  struct RecentResult {
    GlobalSearchServiceResult result_;
  };

  GlobalSearch* engine_;

  // GlobalSearch request ids
  QMap<int, PendingSearch> pending_searches_;

  // Result ids
  QMap<int, RecentResult> recent_results_;
  QMap<int, int> prefetching_art_; // LoadArt id -> result id

  int next_result_id_;
};

#endif // GLOBALSEARCHSERVICE_H
