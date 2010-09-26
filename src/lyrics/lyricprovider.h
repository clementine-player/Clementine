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

#ifndef LYRICPROVIDER_H
#define LYRICPROVIDER_H

#include <QObject>

#include "core/song.h"

class NetworkAccessManager;

class LyricProvider : public QObject {
  Q_OBJECT

public:
  LyricProvider(NetworkAccessManager* network, QObject* parent = 0);

  struct Result {
    Result() : valid(false) {}

    bool valid;
    QString title;
    QString content;
  };

  virtual QString name() const = 0;
  virtual Result Search(const Song& metadata) const = 0;

protected:
  NetworkAccessManager* network_;
};

#endif // LYRICPROVIDER_H
