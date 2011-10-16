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

#ifndef CLEMENTINERUNNER_H
#define CLEMENTINERUNNER_H

#include "src/globalsearch/common.h"

#include <QSemaphore>

#include <Plasma/AbstractRunner>

class OrgClementineplayerGlobalSearchInterface;

class ClementineRunner : public Plasma::AbstractRunner {
  Q_OBJECT

public:
  ClementineRunner(QObject* parent, const QVariantList& args);

  static const char* kDbusService;
  static const char* kDbusPath;

  void match(Plasma::RunnerContext& context);
  void run(const Plasma::RunnerContext& context, const Plasma::QueryMatch& match);

private slots:
  void ResultsAvailable(int id, GlobalSearchServiceResultList results);
  void SearchFinished(int id);
  void ArtLoaded(int result_id, const QByteArray& image_data);

private:
  struct PendingQuery {
    PendingQuery()
      : provider_order_(0),
        finished_signal_emitted_(false),
        finished_(false)
    {}

    // The main thread is the only one to access these variables.
    int provider_order_;
    GlobalSearchServiceResultList results_waiting_for_art_;
    bool finished_signal_emitted_;

    // This list contains results that are finished and waiting to be processed
    // by the match() thread.  results_mutex_ locks results_ and
    // results_semaphore_ is released once for each result.
    GlobalSearchServiceResultList results_;
    QMutex results_mutex_;
    QSemaphore results_semaphore_;
    bool finished_;
  };
  typedef QMap<int, PendingQuery*> PendingMap;

  qreal ResultRelevance(const GlobalSearchServiceResult& result) const;
  void FillMatch(const GlobalSearchServiceResult& result,
                 Plasma::QueryMatch* match) const;

private:
  OrgClementineplayerGlobalSearchInterface* interface_;

  QIcon nocover_;

  // pending_mutex_ locks any access to the PendingMap.
  QMutex pending_mutex_;
  PendingMap pending_queries_;
};

#endif // CLEMENTINERUNNER_H
