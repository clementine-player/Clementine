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

#include "config.h"
#include "projectmpresetmodel.h"
#include "projectmvisualisation.h"

#ifdef USE_SYSTEM_PROJECTM
#include <libprojectM/projectM.hpp>
#else
#include "projectM.hpp"
#endif

#include <QtDebug>
#include <QDir>

ProjectMPresetModel::ProjectMPresetModel(ProjectMVisualisation* vis,
                                         QObject* parent)
    : QAbstractItemModel(parent), vis_(vis) {
  // Find presets
  QDir preset_dir(vis_->preset_url());
  QStringList presets(
      preset_dir.entryList(QStringList() << "*.milk"
                                         << "*.prjm",
                           QDir::Files | QDir::NoDotAndDotDot | QDir::Readable,
                           QDir::Name | QDir::IgnoreCase));

  for (const QString& filename : presets) {
    all_presets_ << Preset(preset_dir.absoluteFilePath(filename), filename,
                           false);
  }
}

int ProjectMPresetModel::rowCount(const QModelIndex&) const {
  if (!vis_) return 0;
  return all_presets_.count();
}

int ProjectMPresetModel::columnCount(const QModelIndex&) const { return 1; }

QModelIndex ProjectMPresetModel::index(int row, int column,
                                       const QModelIndex&) const {
  return createIndex(row, column);
}

QModelIndex ProjectMPresetModel::parent(const QModelIndex& child) const {
  return QModelIndex();
}

QVariant ProjectMPresetModel::data(const QModelIndex& index, int role) const {
  switch (role) {
    case Qt::DisplayRole:
      return all_presets_[index.row()].name_;
    case Qt::CheckStateRole: {
      bool selected = all_presets_[index.row()].selected_;
      return selected ? Qt::Checked : Qt::Unchecked;
    }
    case Role_Url:
      return all_presets_[index.row()].path_;
    default:
      return QVariant();
  }
}

Qt::ItemFlags ProjectMPresetModel::flags(const QModelIndex& index) const {
  if (!index.isValid()) return QAbstractItemModel::flags(index);
  return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsUserCheckable |
         Qt::ItemIsEnabled;
}

bool ProjectMPresetModel::setData(const QModelIndex& index,
                                  const QVariant& value, int role) {
  if (role == Qt::CheckStateRole) {
    all_presets_[index.row()].selected_ = value.toBool();
    vis_->SetSelected(QStringList() << all_presets_[index.row()].path_,
                      value.toBool());
    return true;
  }
  return false;
}

void ProjectMPresetModel::SetImmediatePreset(const QModelIndex& index) {
  vis_->SetImmediatePreset(all_presets_[index.row()].path_);
}

void ProjectMPresetModel::SelectAll() {
  QStringList paths;
  for (int i = 0; i < all_presets_.count(); ++i) {
    paths << all_presets_[i].path_;
    all_presets_[i].selected_ = true;
  }
  vis_->SetSelected(paths, true);

  emit dataChanged(index(0, 0), index(rowCount() - 1, 0));
}

void ProjectMPresetModel::SelectNone() {
  vis_->ClearSelected();
  for (int i = 0; i < all_presets_.count(); ++i) {
    all_presets_[i].selected_ = false;
  }

  emit dataChanged(index(0, 0), index(rowCount() - 1, 0));
}

void ProjectMPresetModel::MarkSelected(const QString& path, bool selected) {
  for (int i = 0; i < all_presets_.count(); ++i) {
    if (path == all_presets_[i].path_) {
      all_presets_[i].selected_ = selected;
      return;
    }
  }
}
