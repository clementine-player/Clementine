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

#ifndef COVERFROMURLDIALOG_H
#define COVERFROMURLDIALOG_H

#include <QDialog>
#include <QImage>

class NetworkAccessManager;
class Song;
class Ui_CoverFromURLDialog;

// Controller for a dialog which fetches covers from the given URL.
class CoverFromURLDialog : public QDialog {
  Q_OBJECT

 public:
  CoverFromURLDialog(QWidget* parent = nullptr);
  ~CoverFromURLDialog();

  // Opens the dialog. This returns an image found at the URL chosen by user
  // or null image if the dialog got rejected.
  QImage Exec();

 private slots:
  void accept();
  void LoadCoverFromURLFinished();

 private:
  Ui_CoverFromURLDialog* ui_;

  NetworkAccessManager* network_;
  QImage last_image_;
};

#endif  // COVERFROMURLDIALOG_H
