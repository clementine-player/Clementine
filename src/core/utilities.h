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

#include <boost/scoped_array.hpp>

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

class ScopedWCharArray {
public:
  ScopedWCharArray(const QString& str);

  QString ToString() const { return QString::fromWCharArray(data_.get()); }

  wchar_t* get() const { return data_.get(); }
  operator wchar_t*() const { return get(); }

  int characters() const { return chars_; }
  int bytes() const { return (chars_ + 1) * sizeof(wchar_t); }

private:
  Q_DISABLE_COPY(ScopedWCharArray);

  int chars_;
  boost::scoped_array<wchar_t> data_;
};

#endif // UTILITIES_H
