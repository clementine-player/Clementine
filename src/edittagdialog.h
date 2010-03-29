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

#ifndef EDITTAGDIALOG_H
#define EDITTAGDIALOG_H

#include <QDialog>

#include "ui_edittagdialog.h"
#include "song.h"

class Library;

class EditTagDialog : public QDialog {
  Q_OBJECT

 public:
  EditTagDialog(QWidget* parent = 0);

  bool SetSongs(const SongList& songs);
  void SetTagCompleter(Library* library);

 public slots:
  void accept();

 signals:
  void SongEdited(const Song& old_song, const Song& new_song);

 private:
  Ui::EditTagDialog ui_;

  SongList songs_;
};

#endif // EDITTAGDIALOG_H
