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

#ifndef REMOTECONFIG_H
#define REMOTECONFIG_H

#include <QWidget>

#include <boost/scoped_ptr.hpp>

#include "core/network.h"

class Ui_RemoteConfig;

class RemoteConfig : public QWidget {
  Q_OBJECT

 public:
  RemoteConfig(QWidget* parent = 0);
  ~RemoteConfig();

  bool NeedsValidation() const;

 public slots:
  void Validate();
  void Load();
  void Save();

 signals:
  void ValidationComplete(bool success);

 private slots:
  void AuthenticationComplete(bool success);
  void SignOut();
  void ValidateFinished();

 private:
  void ValidateGoogleAccount(const QString& username, const QString& password);

  Ui_RemoteConfig* ui_;
  bool waiting_for_auth_;
  boost::scoped_ptr<NetworkAccessManager> network_;
};

#endif // REMOTECONFIG_H
