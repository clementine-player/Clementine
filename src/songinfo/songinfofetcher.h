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

#ifndef SONGINFOFETCHER_H
#define SONGINFOFETCHER_H

#include <QMap>
#include <QObject>
#include <QUrl>

#include "collapsibleinfopane.h"
#include "core/song.h"

class SongInfoProvider;

class QSignalMapper;

class SongInfoFetcher : public QObject {
  Q_OBJECT

 public:
  SongInfoFetcher(QObject* parent = 0);

  struct Result {
    QList<QUrl> images_;
    QList<CollapsibleInfoPane::Data> info_;
  };

  static const int kDefaultTimeoutDuration = 25000;  // msec

  void AddProvider(SongInfoProvider* provider);
  int FetchInfo(const Song& metadata);

  QList<SongInfoProvider*> providers() const { return providers_; }

signals:
  void InfoResultReady(int id, const CollapsibleInfoPane::Data& data);
  void ResultReady(int id, const SongInfoFetcher::Result& result);

 private slots:
  void ImageReady(int id, const QUrl& url);
  void InfoReady(int id, const CollapsibleInfoPane::Data& data);
  void ProviderFinished(int id);
  void Timeout(int id);

 private:
  QList<SongInfoProvider*> providers_;

  QMap<int, Result> results_;
  QMap<int, QList<SongInfoProvider*> > waiting_for_;
  QMap<int, QTimer*> timeout_timers_;

  QSignalMapper* timeout_timer_mapper_;
  int timeout_duration_;

  int next_id_;
};

#endif  // SONGINFOFETCHER_H
