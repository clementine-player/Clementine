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

#include "trackselectiondialog.h"

#include <QtDebug>

TrackSelectionDialog::TrackSelectionDialog(QWidget *parent)
  : QDialog(parent),
    ui_(new Ui_TrackSelectionDialog)
{
  // Setup dialog window
  ui_->setupUi(this);
  setModal(true);
  connect(ui_->resultsTreeWidget, SIGNAL(itemDoubleClicked (QTreeWidgetItem*, int)), this, SLOT(accept()));
}

TrackSelectionDialog::~TrackSelectionDialog() {
  Clean();
  delete ui_;
}

void TrackSelectionDialog::Init(const QString& filename, const SongList& songs) {
  current_filename_ = filename;
  current_songs_ = songs;

  // Set filename
  ui_->filenameLabel->setText(current_filename_); //TODO: use basefilename, it's nicer

  // Fill tree view with songs
  int song_index = 0;
  foreach(const Song& song, songs) {
    QStringList valuesStringList;
    valuesStringList << song.title() << song.artist() << song.album();
    if (song.track() > 0) {
      valuesStringList << QString::number(song.track());
    }
    QTreeWidgetItem* item = new QTreeWidgetItem(
        ui_->resultsTreeWidget, valuesStringList);
    if(song_index==0) { // if it's the first item, set focus on it
      ui_->resultsTreeWidget->setCurrentItem(item);
    }
    item->setData(0,  Qt::UserRole, song_index++); // To remember this widget corresponds to the ith song of our list
  }

  // Resize columns
  ui_->resultsTreeWidget->setColumnWidth(0, 175); // Title column
  ui_->resultsTreeWidget->setColumnWidth(1, 175); // Artist column
  ui_->resultsTreeWidget->setColumnWidth(2, 175); // Album column
  ui_->resultsTreeWidget->setColumnWidth(3, 50);  // Track column
}

void TrackSelectionDialog::Clean() {
  // Remove and deleted all items
  ui_->resultsTreeWidget->clear();
}

void TrackSelectionDialog::accept() {
  QDialog::accept();
  QTreeWidgetItem *item =ui_->resultsTreeWidget->currentItem();
  if(!item) { // no item selected
    return;
  }
  int selected_song_index =  item->data(0, Qt::UserRole).toInt();
  if(selected_song_index >= 0 && selected_song_index < current_songs_.size()) {
    emit SongChoosen(current_filename_, current_songs_[selected_song_index]);
  }
}

