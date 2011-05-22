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
  CoverProvider(const QString& name, QObject* parent = &CoverProviders::instance());
  virtual ~CoverProvider() {}

  // A name (very short description) of this provider, like "last.fm".
  QString name() const { return name_; }

  // Given a search request from Clementine, provider has to create and invoke
  // a NetworkRequest. It then has to return a corresponding NetworkReply,
  // without connecting to its finished() signal!
  // Responsibilities of provider:
  // - maps the given query to a NetworkRequest that a service this provider
  //   uses will understand
  // - makes the prepared request and returns the resulting reply
  virtual QNetworkReply* SendRequest(const QString& query) = 0;

  // Provider parses a reply which is now filled with data obtained from a service
  // this provider communicates with. The result is a QList of CoverSearchResult
  // objects.
  virtual CoverSearchResults ParseReply(QNetworkReply* reply) = 0;

private:
  QString name_;
};

#endif // COVERPROVIDER_H
