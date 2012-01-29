/* This file is part of Clementine.
   Copyright 2012, Martin Björklund <mbj4668@gmail.com>

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

#ifndef DISCOGSCOVERPROVIDER_H
#define DISCOGSCOVERPROVIDER_H

#include "coverprovider.h"
#include <QVariant>

class QNetworkAccessManager;


class DiscogsCoverProvider : public CoverProvider {
  Q_OBJECT

public:
  DiscogsCoverProvider(QObject* parent = NULL);

  static const char* kSearchUrl;

  bool StartSearch(const QString& artist, const QString& album, int id);

private slots:
  void SearchReply(QNetworkReply* reply, int id);
  void QueryResourceReply(QNetworkReply* reply, int id, const QVariantList& results);

private:
  QNetworkAccessManager* network_;
  void QueryResource(const QString& resource_url, int id, const QVariantList& results);
  void FollowBestResult(QNetworkReply* reply, int id, QVariantList results);
};

#endif // DISCOGSCOVERPROVIDER_H
