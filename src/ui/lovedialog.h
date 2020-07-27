/* This file is part of Clementine.
   Copyright 2020, Dave McKellar

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

#ifndef LOVE_DIALOG_H
#define LOVE_DIALOG_H

#include <QDialog>

#include "core/song.h"

class Application;
class Ui_LoveDialog;

class QAbstractButton;
class QLabel;

class LoveDialog : public QDialog {
  Q_OBJECT

 public:
  LoveDialog(Application* app, QWidget* parent = nullptr);
  ~LoveDialog();

  static const char* kSettingsGroup;

  void SetSong(const Song&);

signals:
  void Error(const QString& message);

 protected:
  void showEvent(QShowEvent*);

 private slots:
  void ButtonClicked(QAbstractButton* button);
  void SongRated(float rating);

 private:
  Ui_LoveDialog* ui_;
  Application* app_;
  Song song_;
};

#endif
