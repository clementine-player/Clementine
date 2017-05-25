/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>
   Copyright 2015 - 2016, Arun Narayanankutty <n.arun.lifescience@gmail.com>

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

#ifndef ICONLOADER_H
#define ICONLOADER_H

#include <QIcon>

class IconLoader {
 public:
  enum IconType {
    Base = 0,
    Provider = 1,
    Lastfm = 2,
    Other = 3
  };

  static void Init();
  static QIcon Load(const QString& name, const IconType& icontype);

 private:
  IconLoader() {}

  static QList<int> sizes_;
  static QString custom_icon_path_;
  static QList<QString> icon_sub_path_;
  static bool use_sys_icons_;
};

#endif  // ICONLOADER_H
