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

#ifndef SCRIPTINTERFACE_H
#define SCRIPTINTERFACE_H

#include <QObject>

#include <cstdlib>

class Script;

// An instance of this class is made available to each script
class ScriptInterface : public QObject {
  Q_OBJECT

public:
  ScriptInterface(Script* script, QObject* parent = 0);

  // Gdb will break here
  void Abort();

public slots:
  // Callable by C++
  void ShowSettingsDialog();

signals:
  // Scripts should connect to this and show a settings dialog
  void SettingsDialogRequested();

private:
  Script* script_;
};

#endif // SCRIPTINTERFACE_H
