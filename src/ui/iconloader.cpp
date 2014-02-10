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

#include "iconloader.h"
#include "core/logging.h"

#include <QFile>
#include <QtDebug>

QList<int> IconLoader::sizes_;

void IconLoader::Init() {
  sizes_.clear();
  sizes_ << 22 << 32 << 48;
}

QIcon IconLoader::Load(const QString& name) {
  QIcon ret;

  if (name.isEmpty()) return ret;

#if QT_VERSION >= 0x040600
  // Try to load it from the theme initially
  ret = QIcon::fromTheme(name);
  if (!ret.isNull()) return ret;
#endif

  // Otherwise use our fallback theme
  const QString path(":/icons/%1x%2/%3.png");
  for (int size : sizes_) {
    QString filename(path.arg(size).arg(size).arg(name));

    if (QFile::exists(filename)) ret.addFile(filename, QSize(size, size));
  }

  if (ret.isNull()) qLog(Warning) << "Couldn't load icon" << name;
  return ret;
}
