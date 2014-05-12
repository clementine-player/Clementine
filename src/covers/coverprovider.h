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

#ifndef COVERPROVIDER_H
#define COVERPROVIDER_H

#include "albumcoverfetcher.h"
#include "coverproviders.h"

#include <QObject>

class QNetworkReply;

// Each implementation of this interface downloads covers from one online
// service. There are no limitations on what this service might be - last.fm,
// Amazon, Google Images - you name it.
class CoverProvider : public QObject {
  Q_OBJECT

 public:
  CoverProvider(const QString& name, QObject* parent);

  // A name (very short description) of this provider, like "last.fm".
  QString name() const { return name_; }

  // Starts searching for covers matching the given query text.  Returns true
  // if the query has been started, or false if an error occurred.  The provider
  // should remember the ID and emit it along with the result when it finishes.
  virtual bool StartSearch(const QString& artist, const QString& album,
                           int id) = 0;

  virtual void CancelSearch(int id) {}

signals:
  void SearchFinished(int id, const QList<CoverSearchResult>& results);

 private:
  QString name_;
};

#endif  // COVERPROVIDER_H
