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

#include "libraryfilterwidget.h"
#include "librarymodel.h"
#include "savedgroupingmanager.h"
#include "ui_savedgroupingmanager.h"
#include "ui/iconloader.h"

#include <QKeySequence>
#include <QList>
#include <QSettings>
#include <QStandardItem>

SavedGroupingManager::SavedGroupingManager(QWidget* parent)
    : QDialog(parent),
      ui_(new Ui_SavedGroupingManager),
      model_(new QStandardItemModel(0, 4, this)) {
  ui_->setupUi(this);

  model_->setHorizontalHeaderItem(0, new QStandardItem(tr("Name")));
  model_->setHorizontalHeaderItem(1, new QStandardItem(tr("First level")));
  model_->setHorizontalHeaderItem(2, new QStandardItem(tr("Second Level")));
  model_->setHorizontalHeaderItem(3, new QStandardItem(tr("Third Level")));
  ui_->list->setModel(model_);
  ui_->remove->setIcon(IconLoader::Load("edit-delete", IconLoader::Base));
  ui_->remove->setEnabled(false);

  ui_->remove->setShortcut(QKeySequence::Delete);
  connect(ui_->list->selectionModel(),
          SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
          SLOT(UpdateButtonState()));

  connect(ui_->remove, SIGNAL(clicked()), SLOT(Remove()));
}

SavedGroupingManager::~SavedGroupingManager() {
  delete ui_;
  delete model_;
}

QString SavedGroupingManager::GroupByToString(const LibraryModel::GroupBy& g) {
  switch (g) {
    case LibraryModel::GroupBy_None: {
      return tr("None");
    }
    case LibraryModel::GroupBy_Artist: {
      return tr("Artist");
    }
    case LibraryModel::GroupBy_Album: {
      return tr("Album");
    }
    case LibraryModel::GroupBy_YearAlbum: {
      return tr("Year - Album");
    }
    case LibraryModel::GroupBy_Year: {
      return tr("Year");
    }
    case LibraryModel::GroupBy_Composer: {
      return tr("Composer");
    }
    case LibraryModel::GroupBy_Genre: {
      return tr("Genre");
    }
    case LibraryModel::GroupBy_AlbumArtist: {
      return tr("Album artist");
    }
    case LibraryModel::GroupBy_FileType: {
      return tr("File type");
    }
    case LibraryModel::GroupBy_Performer: {
      return tr("Performer");
    }
    case LibraryModel::GroupBy_Grouping: {
      return tr("Grouping");
    }
    case LibraryModel::GroupBy_Bitrate: {
      return tr("Bitrate");
    }
    case LibraryModel::GroupBy_Disc: {
      return tr("Disc");
    }
    case LibraryModel::GroupBy_OriginalYearAlbum: {
      return tr("Original year - Album");
    }
    case LibraryModel::GroupBy_OriginalYear: {
      return tr("Original year");
    }
    default: { return tr("Unknown"); }
  }
}

void SavedGroupingManager::UpdateModel() {
  model_->setRowCount(0);  // don't use clear, it deletes headers
  QSettings s;
  s.beginGroup(LibraryModel::kSavedGroupingsSettingsGroup);
  QStringList saved = s.childKeys();
  for (int i = 0; i < saved.size(); ++i) {
    QByteArray bytes = s.value(saved.at(i)).toByteArray();
    QDataStream ds(&bytes, QIODevice::ReadOnly);
    LibraryModel::Grouping g;
    ds >> g;

    QList<QStandardItem*> list;
    list << new QStandardItem(saved.at(i))
         << new QStandardItem(GroupByToString(g.first))
         << new QStandardItem(GroupByToString(g.second))
         << new QStandardItem(GroupByToString(g.third));

    model_->appendRow(list);
  }
}

void SavedGroupingManager::Remove() {
  if (ui_->list->selectionModel()->hasSelection()) {
    QSettings s;
    s.beginGroup(LibraryModel::kSavedGroupingsSettingsGroup);
    for (const QModelIndex& index :
         ui_->list->selectionModel()->selectedRows()) {
      if (index.isValid()) {
        qLog(Debug) << "Remove saved grouping: "
                    << model_->item(index.row(), 0)->text();
        s.remove(model_->item(index.row(), 0)->text());
      }
    }
  }
  UpdateModel();
  filter_->UpdateGroupByActions();
}

void SavedGroupingManager::UpdateButtonState() {
  if (ui_->list->selectionModel()->hasSelection()) {
    const QModelIndex current = ui_->list->selectionModel()->currentIndex();
    ui_->remove->setEnabled(current.isValid());
  } else {
    ui_->remove->setEnabled(false);
  }
}
