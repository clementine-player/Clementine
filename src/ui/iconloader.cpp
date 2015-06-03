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

#include "iconloader.h"
#include "core/appearance.h"
#include "core/logging.h"
#include "core/utilities.h"

#include <QFile>
#include <QtDebug>
#include <QDir>
#include <QSettings>

QList<int> IconLoader::sizes_;
QString IconLoader::custom_icon_path_;
QList<QString> IconLoader::icon_sub_path_;
bool IconLoader::use_sys_icons_;

void IconLoader::Init() {
  sizes_.clear();
  sizes_ << 22 << 32 << 48;
  custom_icon_path_ = Utilities::GetConfigPath(Utilities::Path_Icons);
  icon_sub_path_.clear();
  icon_sub_path_ << "/icons" << "/providers" << "/last.fm" << "";
  QSettings settings;
  settings.beginGroup(Appearance::kSettingsGroup);
  use_sys_icons_ = settings.value("b_use_sys_icons", false).toBool();
}

QIcon IconLoader::Load(const QString& name, const IconType& icontype) {
  QIcon ret;
  // If the icon name is empty
  if (name.isEmpty()) {
    qLog(Warning) << "Icon name is null";
    return ret;
  }

  if (use_sys_icons_) {
    ret = QIcon::fromTheme(name);
    if (!ret.isNull()) return ret;
  }

  // Set the icon load location based on IConType
  switch (icontype) {
  case Base:
  case Provider: {
    const QString custom_icon_location = custom_icon_path_
        + icon_sub_path_.at(icontype);
    if (QDir(custom_icon_location).exists()) {
      // Try to load icons from the custom icon location initially
      const QString locate(custom_icon_location + "/%1x%2/%3.png");
      for (int size : sizes_) {
        QString filename_custom(locate.arg(size).arg(size).arg(name));

        if (QFile::exists(filename_custom)) ret.addFile(filename_custom,
                                                        QSize(size, size));
      }
      if (!ret.isNull()) return ret;
    }

    // Otherwise use our fallback theme
    const QString path(":" + icon_sub_path_.at(icontype) + "/%1x%2/%3.png");
    for (int size : sizes_) {
      QString filename(path.arg(size).arg(size).arg(name));

      if (QFile::exists(filename)) ret.addFile(filename, QSize(size, size));
    }
    break;
  }

  case Lastfm:
  case Other: {
    // lastfm icons location
    const QString custom_fm_other_icon_location = custom_icon_path_
        + icon_sub_path_.at(icontype);
    if (QDir(custom_fm_other_icon_location).exists()) {
      // Try to load icons from the custom icon location initially
      const QString locate_file(
          custom_fm_other_icon_location + "/" + name + ".png");

      if (QFile::exists(locate_file)) ret.addFile(locate_file);
      if (!ret.isNull()) return ret;
    }

    // Otherwise use our fallback theme
    const QString path_file(":" + icon_sub_path_.at(icontype)
        + "/" + name + ".png");

    if (QFile::exists(path_file)) ret.addFile(path_file);
    break;
  }

  default:
    // Should never be reached
    qLog(Warning) << "Couldn't recognize IconType" << name;
  }

    // Load icon from system theme only if it hasn't been found
  if (ret.isNull()) {
    ret = QIcon::fromTheme(name);
    if (!ret.isNull()) return ret;
    qLog(Warning) << "Couldn't load icon" << name;
  }
  else qLog(Warning) << "Couldn't load icon" << name;

  return ret;
}
