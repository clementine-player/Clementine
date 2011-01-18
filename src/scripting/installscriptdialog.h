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

#ifndef INSTALLSCRIPTDIALOG_H
#define INSTALLSCRIPTDIALOG_H

#include <QDialog>

class ScriptArchive;
class Ui_InstallScriptDialog;

class InstallScriptDialog : public QDialog {
  Q_OBJECT

public:
  // The dialog will take ownership of the archive and will destroy the archive
  // *and itself* when it is closed.
  InstallScriptDialog(ScriptArchive* archive, QWidget* parent = 0);
  ~InstallScriptDialog();

public slots:
  void accept();

private:
  Ui_InstallScriptDialog* ui_;

  ScriptArchive* archive_;
};

#endif // INSTALLSCRIPTDIALOG_H
