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

#include "filesystemmusicstorage.h"

#include <QDir>
#include <QFile>

FilesystemMusicStorage::FilesystemMusicStorage(const QString& root)
  : root_(root)
{
}

bool FilesystemMusicStorage::CopyToStorage(const CopyJob& job) {
  const QString dest_filename = root_ + "/" + job.destination_;

  // Don't do anything if the destination is the same as the source
  if (job.source_ == dest_filename)
    return true;

  // Create directories as required
  QDir dir;
  dir.mkpath(dest_filename.section('/', 0, -2));

  // Remove the destination file if it exists and we want to overwrite
  if (job.overwrite_ && QFile::exists(dest_filename))
    QFile::remove(dest_filename);

  // Copy or move
  if (job.remove_original_)
    return QFile::rename(job.source_, dest_filename);
  else
    return QFile::copy(job.source_, dest_filename);
}

bool FilesystemMusicStorage::DeleteFromStorage(const DeleteJob& job) {
  return QFile::remove(job.metadata_.filename());
}
