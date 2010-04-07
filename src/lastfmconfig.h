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

#ifndef LASTFMCONFIG_H
#define LASTFMCONFIG_H

#include <QWidget>

#include "ui_lastfmconfig.h"

class LastFMService;

class LastFMConfig : public QWidget {
  Q_OBJECT

 public:
  LastFMConfig(QWidget* parent = 0);

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

 private:
  LastFMService* service_;
  Ui::LastFMConfig ui_;
};

#endif // LASTFMCONFIG_H
