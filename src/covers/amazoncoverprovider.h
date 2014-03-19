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

#ifndef AMAZONCOVERPROVIDER_H
#define AMAZONCOVERPROVIDER_H

#include "coverprovider.h"

#include <QXmlStreamReader>

class QNetworkAccessManager;

class AmazonCoverProvider : public CoverProvider {
  Q_OBJECT

 public:
  AmazonCoverProvider(QObject* parent = nullptr);

  static const char* kAccessKeyB64;
  static const char* kSecretAccessKeyB64;
  static const char* kUrl;
  static const char* kAssociateTag;

  bool StartSearch(const QString& artist, const QString& album, int id);

 private slots:
  void QueryFinished(QNetworkReply* reply, int id);

 private:
  void ReadItem(QXmlStreamReader* reader, CoverSearchResults* results);
  void ReadLargeImage(QXmlStreamReader* reader, CoverSearchResults* results);

 private:
  QNetworkAccessManager* network_;
};

#endif  // AMAZONCOVERPROVIDER_H
