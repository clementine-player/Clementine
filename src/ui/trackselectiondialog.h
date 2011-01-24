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

#ifndef TRACKSELECTIONDIALOG_H
#define TRACKSELECTIONDIALOG_H

#include <QDialog>

#include "config.h"

#include "core/song.h"
#include "ui_trackselectiondialog.h"

class TrackSelectionDialog : public QDialog {
  Q_OBJECT

public:
  TrackSelectionDialog(QWidget *parent = 0);
  ~TrackSelectionDialog();

  void Init(const QString& filename, const SongList& songs);

  void accept();

signals:
  void SongChoosen(const QString& filename, const Song& song);

private:
  Ui_TrackSelectionDialog *ui_;

  QString   current_filename_;
  SongList  current_songs_;
};

#endif // TRACKSELECTIONDIALOG_H


