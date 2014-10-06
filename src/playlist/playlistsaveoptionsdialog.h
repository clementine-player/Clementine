/* This file is part of Clementine.
   Copyright 2014, David Sansome <me@davidsansome.com>

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

#ifndef PLAYLISTSAVEOPTIONSDIALOG_H
#define PLAYLISTSAVEOPTIONSDIALOG_H

#include <QDialog>

#include "playlist.h"

namespace Ui {
class PlaylistSaveOptionsDialog;
}

class PlaylistSaveOptionsDialog : public QDialog {
  Q_OBJECT

 public:
  explicit PlaylistSaveOptionsDialog(QWidget* parent = 0);
  ~PlaylistSaveOptionsDialog();

  void accept();
  Playlist::Path path_type() const;

 private:
  static const char* kSettingsGroup;

  Ui::PlaylistSaveOptionsDialog* ui;
};

#endif  // PLAYLISTSAVEOPTIONSDIALOG_H
