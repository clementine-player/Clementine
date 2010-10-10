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

#ifndef LYRICSETTINGS_H
#define LYRICSETTINGS_H

#include <QWidget>

class Ui_LyricSettings;

class QListWidgetItem;

class LyricSettings : public QWidget {
  Q_OBJECT

public:
  LyricSettings(QWidget *parent = 0);
  ~LyricSettings();

  void Load();
  void Save();

private slots:
  void MoveUp();
  void MoveDown();
  void Move(int d);

  void CurrentItemChanged(QListWidgetItem* item);
  void ItemChanged(QListWidgetItem* item);

private:
  Ui_LyricSettings* ui_;
};

#endif // LYRICSETTINGS_H
