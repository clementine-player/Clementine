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

#include "devicemanager.h"

#include <memory>

#include <QIcon>
#include <QPixmap>
#include <QString>
#include <QStringList>
#include <QVariant>

#include "config.h"
#include "core/logging.h"
#include "core/simpletreemodel.h"
#include "ui/iconloader.h"

#include "devicedatabasebackend.h"
#include "deviceinfo.h"
#include "devicelister.h"

DeviceDatabaseBackend::Device DeviceInfo::SaveToDb() const {
  DeviceDatabaseBackend::Device ret;
  ret.friendly_name_ = friendly_name_;
  ret.size_ = size_;
  ret.id_ = database_id_;
  ret.icon_name_ = icon_name_;
  ret.transcode_mode_ = transcode_mode_;
  ret.transcode_format_ = transcode_format_;

  QStringList unique_ids;
  for (const Backend& backend : backends_) {
    unique_ids << backend.unique_id_;
  }
  ret.unique_id_ = unique_ids.join(",");

  return ret;
}

void DeviceInfo::InitFromDb(const DeviceDatabaseBackend::Device& dev) {
  database_id_ = dev.id_;
  friendly_name_ = dev.friendly_name_;
  size_ = dev.size_;
  transcode_mode_ = dev.transcode_mode_;
  transcode_format_ = dev.transcode_format_;

  QStringList icon_names = dev.icon_name_.split(',');
  QVariantList icons;
  for (const QString& icon_name : icon_names) {
    icons << icon_name;
  }

  LoadIcon(icons, friendly_name_);

  QStringList unique_ids = dev.unique_id_.split(',');
  for (const QString& id : unique_ids) {
    backends_ << Backend(nullptr, id);
  }
}

const DeviceInfo::Backend* DeviceInfo::BestBackend() const {
  int best_priority = -1;
  const Backend* ret = nullptr;

  for (int i = 0; i < backends_.count(); ++i) {
    if (backends_[i].lister_ &&
        backends_[i].lister_->priority() > best_priority) {
      best_priority = backends_[i].lister_->priority();
      ret = &(backends_[i]);
    }
  }

  if (!ret && !backends_.isEmpty()) return &(backends_[0]);
  return ret;
}

void DeviceInfo::LoadIcon(const QVariantList& icons, const QString& name_hint) {
  if (icons.isEmpty()) {
    icon_name_ = "drive-removable-media-usb-pendrive";
    icon_ = IconLoader::Load(icon_name_, IconLoader::Base);
    return;
  }

  // Try to load the icon with that exact name first
  for (const QVariant& icon : icons) {
    if (!icon.value<QPixmap>().isNull()) {
      icon_ = QIcon(icon.value<QPixmap>());
      return;
    } else {
      icon_ = IconLoader::Load(icon.toString(), IconLoader::Base);
      if (!icon_.isNull()) {
        icon_name_ = icon.toString();
        return;
      }
    }
  }

  QString hint = QString(icons.first().toString() + name_hint).toLower();

  // If that failed than try to guess if it's a phone or ipod.  Fall back on
  // a usb memory stick icon.
  if (hint.contains("phone"))
    icon_name_ = "phone";
  else if (hint.contains("ipod") || hint.contains("apple"))
    icon_name_ = "multimedia-player-ipod-standard-monochrome";
  else
    icon_name_ = "drive-removable-media-usb-pendrive";
  icon_ = IconLoader::Load(icon_name_, IconLoader::Base);
}
