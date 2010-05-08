/* This file is part of Clementine.

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

#ifndef MAGNATUNESERVICE_H
#define MAGNATUNESERVICE_H

#include <QXmlStreamReader>

#include "radioservice.h"

class QNetworkAccessManager;

class MagnatuneService : public RadioService {
  Q_OBJECT

 public:
  MagnatuneService(QObject* parent = 0);

  static const char* kServiceName;
  static const char* kDatabaseUrl;

  RadioItem* CreateRootItem(RadioItem* parent);
  void LazyPopulate(RadioItem* item);

  void StartLoading(const QUrl &url);

 private slots:
  void ReloadDatabase();
  void ReloadDatabaseFinished();

 private:
  void ReadTrack(QXmlStreamReader& reader);

 private:
  RadioItem* root_;

  QNetworkAccessManager* network_;
};

#endif // MAGNATUNESERVICE_H
