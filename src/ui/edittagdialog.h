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
#include <QFutureWatcher>
#include <QMutex>

#include "core/song.h"

class LibraryBackend;
class Ui_EditTagDialog;

class EditTagDialog : public QDialog {
  Q_OBJECT

 public:
  EditTagDialog(QWidget* parent = 0);
  ~EditTagDialog();

  static const char* kHintText;

  bool SetSongs(const SongList& songs);
  void SetTagCompleter(LibraryBackend* backend);

 public slots:
  void accept();

 private slots:
  void SongsEdited();

 private:
  void SaveSong(const Song& song);

  Ui_EditTagDialog* ui_;

  SongList songs_;

  QString common_artist_;
  QString common_album_;
  QString common_genre_;
  int common_year_;

  QFutureWatcher<void> watcher_;
  QMutex taglib_mutex_;
};

#endif // EDITTAGDIALOG_H
