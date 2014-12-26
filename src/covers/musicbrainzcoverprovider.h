/* This file is part of Clementine.
   Copyright 2012, John Maguire <john.maguire@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>

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

#ifndef COVERS_MUSICBRAINZCOVERPROVIDER_H_
#define COVERS_MUSICBRAINZCOVERPROVIDER_H_

#include "coverprovider.h"

#include <QMultiMap>

class QNetworkAccessManager;
class QNetworkReply;

class MusicbrainzCoverProvider : public CoverProvider {
  Q_OBJECT
 public:
  explicit MusicbrainzCoverProvider(QObject* parent = nullptr);

  // CoverProvider
  virtual bool StartSearch(const QString& artist, const QString& album, int id);
  virtual void CancelSearch(int id);

 private slots:
  void ReleaseSearchFinished(QNetworkReply* reply, int id);
  void ImageCheckFinished(int id);

 private:
  QNetworkAccessManager* network_;
  QMultiMap<int, QNetworkReply*> image_checks_;
  QMap<int, QString> cover_names_;
};

#endif  // COVERS_MUSICBRAINZCOVERPROVIDER_H_
