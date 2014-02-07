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

#ifndef PROJECTMPRESETMODEL_H
#define PROJECTMPRESETMODEL_H

#include <QAbstractItemModel>

class ProjectMVisualisation;

class ProjectMPresetModel : public QAbstractItemModel {
  Q_OBJECT

  friend class ProjectMVisualisation;

 public:
  ProjectMPresetModel(ProjectMVisualisation* vis, QObject* parent = 0);

  enum { Role_Url = Qt::UserRole, };

  void MarkSelected(const QString& path, bool selected);

  // QAbstractItemModel
  QModelIndex index(int row, int column,
                    const QModelIndex& parent = QModelIndex()) const;
  QModelIndex parent(const QModelIndex& child) const;
  int rowCount(const QModelIndex& parent = QModelIndex()) const;
  int columnCount(const QModelIndex& parent = QModelIndex()) const;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
  Qt::ItemFlags flags(const QModelIndex& index) const;
  bool setData(const QModelIndex& index, const QVariant& value,
               int role = Qt::EditRole);

 public slots:
  void SetImmediatePreset(const QModelIndex& index);
  void SelectAll();
  void SelectNone();

 private:
  struct Preset {
    Preset(const QString& path, const QString& name, bool selected)
        : path_(path), name_(name), selected_(selected) {}

    QString path_;
    QString name_;
    bool selected_;
  };

  ProjectMVisualisation* vis_;
  QList<Preset> all_presets_;
};

#endif  // PROJECTMPRESETMODEL_H
