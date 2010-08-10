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

#ifndef MUSICSTORAGE_H
#define MUSICSTORAGE_H

#include "song.h"

#include <QMetaType>

#include <boost/shared_ptr.hpp>

class MusicStorage {
public:
  MusicStorage();
  virtual ~MusicStorage() {}

  enum Role {
    Role_Storage = Qt::UserRole + 100,
    Role_Capacity,
    Role_FreeSpace,
  };

  virtual QString LocalPath() const { return QString(); }

  virtual void StartCopy() {}
  virtual bool CopyToStorage(const QString& source, const QString& destination,
                             const Song& metadata, bool overwrite,
                             bool remove_original) = 0;
  virtual void FinishCopy(bool success) {}

  virtual void StartDelete() {}
  virtual bool DeleteFromStorage(const Song& metadata) = 0;
  virtual void FinishDelete(bool success) {}

  virtual void Eject() {}
};

Q_DECLARE_METATYPE(MusicStorage*);
Q_DECLARE_METATYPE(boost::shared_ptr<MusicStorage>);

#endif // MUSICSTORAGE_H
