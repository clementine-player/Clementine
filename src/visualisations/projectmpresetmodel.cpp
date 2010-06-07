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

#include "projectmpresetmodel.h"
#include "projectmvisualisation.h"

#include <projectM.hpp>

#include <QtDebug>

ProjectMPresetModel::ProjectMPresetModel(ProjectMVisualisation* vis, QObject *parent)
  : QAbstractItemModel(parent),
    vis_(vis)
{
}

int ProjectMPresetModel::rowCount(const QModelIndex&) const {
  if (!vis_)
    return 0;
  return vis_->projectm()->getPlaylistSize();
}

int ProjectMPresetModel::columnCount(const QModelIndex&) const {
  return 1;
}

QModelIndex ProjectMPresetModel::index(int row, int column, const QModelIndex&) const {
  return createIndex(row, column);
}

QModelIndex ProjectMPresetModel::parent(const QModelIndex &child) const {
  return QModelIndex();
}

QVariant ProjectMPresetModel::data(const QModelIndex &index, int role) const {
  switch (role) {
  case Qt::DisplayRole:
    return QString::fromStdString(vis_->projectm()->getPresetName(index.row()));
  case Qt::CheckStateRole: {
    bool selected = vis_->is_selected(index.row());
    return selected ? Qt::Checked : Qt::Unchecked;
  }
  case Role_Url:
    return QString::fromStdString(vis_->projectm()->getPresetURL(index.row()));
  default:
    return QVariant();
  }
}

Qt::ItemFlags ProjectMPresetModel::flags(const QModelIndex &index) const {
  if (!index.isValid())
    return QAbstractItemModel::flags(index);
  return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsUserCheckable |
         Qt::ItemIsEnabled;
}

bool ProjectMPresetModel::setData(const QModelIndex &index,
                                  const QVariant &value, int role) {
  if (role == Qt::CheckStateRole) {
    vis_->set_selected(index.row(), value.toBool());
    return true;
  }
  return false;
}

void ProjectMPresetModel::SetImmediatePreset(const QModelIndex& index) {
  vis_->projectm()->selectPreset(index.row(), true);
}

void ProjectMPresetModel::SelectAll() {
  vis_->set_all_selected(true);
  emit dataChanged(index(0, 0), index(rowCount()-1, 0));
}

void ProjectMPresetModel::SelectNone() {
  vis_->set_all_selected(false);
  emit dataChanged(index(0, 0), index(rowCount()-1, 0));
}

