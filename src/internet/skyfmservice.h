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

#ifndef SKYFMSERVICE_H
#define SKYFMSERVICE_H

#include "digitallyimportedservicebase.h"

class SkyFmService : public DigitallyImportedServiceBase {
  Q_OBJECT

public:
  SkyFmService(InternetModel* model, QObject* parent = NULL);

  void LoadStation(const QString& key);

private:
  void LoadPlaylist(const QString& key, const QString& hash_key = QString());

private slots:
  void LoadHashKeyFinished();

private:
  QString last_key_;
};

#endif // SKYFMSERVICE_H
