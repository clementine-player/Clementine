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

#ifndef UTILITIES_H
#define UTILITIES_H

#include <QString>

class QIODevice;

namespace Utilities {
  QString PrettyTime(int seconds);
  QString PrettySize(quint64 bytes);
  QString WordyTime(quint64 seconds);

  quint64 FileSystemCapacity(const QString& path);
  quint64 FileSystemFreeSpace(const QString& path);

  QString MakeTempDir();
  void RemoveRecursive(const QString& path);
  bool Copy(QIODevice* source, QIODevice* destination);
}

#endif // UTILITIES_H
