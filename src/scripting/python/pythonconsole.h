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

#ifndef PYTHONCONSOLE_H
#define PYTHONCONSOLE_H

#include <QWidget>


class QSortFilterProxyModel;

class PythonQtScriptingConsole;

class PythonEngine;
class Ui_PythonConsole;

class PythonConsole : public QWidget {
  Q_OBJECT

public:
  PythonConsole(QWidget* parent = 0);
  ~PythonConsole();

  void SetEngine(PythonEngine* engine);

protected:
  void showEvent(QShowEvent* e);

private slots:
  void ModuleChanged(int row);
  void ClearConsole();

private:
  Ui_PythonConsole* ui_;
  QSortFilterProxyModel* proxy_model_;

  PythonEngine* engine_;

  PythonQtScriptingConsole* widget_;
};

#endif // PYTHONCONSOLE_H
