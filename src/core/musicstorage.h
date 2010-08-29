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

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

class MusicStorage {
public:
  MusicStorage();
  virtual ~MusicStorage() {}

  enum Role {
    Role_Storage = Qt::UserRole + 100,
    Role_StorageForceConnect,
    Role_Capacity,
    Role_FreeSpace,
  };

  // Values are saved in the database - don't change
  enum TranscodeMode {
    Transcode_Always = 1,
    Transcode_Never = 2,
    Transcode_Unsupported = 3,
  };

  typedef boost::function<void (float progress)> ProgressFunction;

  struct CopyJob {
    QString source_;
    QString destination_;
    Song metadata_;
    bool overwrite_;
    bool remove_original_;
    ProgressFunction progress_;
  };

  struct DeleteJob {
    Song metadata_;
  };

  virtual QString LocalPath() const { return QString(); }

  virtual TranscodeMode GetTranscodeMode() const { return Transcode_Never; }
  virtual Song::FileType GetTranscodeFormat() const { return Song::Type_Unknown; }
  virtual QList<Song::FileType> SupportedFiletypes() { return QList<Song::FileType>(); }

  virtual void StartCopy() {}
  virtual bool CopyToStorage(const CopyJob& job) = 0;
  virtual void FinishCopy(bool success) {}

  virtual void StartDelete() {}
  virtual bool DeleteFromStorage(const DeleteJob& job) = 0;
  virtual void FinishDelete(bool success) {}

  virtual void Eject() {}
};

Q_DECLARE_METATYPE(MusicStorage*);
Q_DECLARE_METATYPE(boost::shared_ptr<MusicStorage>);

#endif // MUSICSTORAGE_H
