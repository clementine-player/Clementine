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

#ifndef PYTHONSCRIPT_H
#define PYTHONSCRIPT_H

#include "scripting/script.h"

class PythonEngine;
class ScriptInfo;

class PythonScript : public Script {
public:
  PythonScript(PythonEngine* engine, const ScriptInfo& info);

  const QString& module_name() const { return module_name_; }
  PythonQtObjectPtr module() const { return module_; }

  bool Init();
  bool Unload();

  void RegisterSignalConnection(PythonQtSignalReceiver* receiver,
                                int signal_id, PyObject* callable);

private:
  PythonEngine* engine_;

  QString module_name_;
  PythonQtObjectPtr module_;

  struct SignalConnection {
    SignalConnection(PythonQtSignalReceiver* receiver,
                     int signal_id, PyObject* callable)
      : receiver_(receiver), signal_id_(signal_id), callable_(callable) {}

    PythonQtSignalReceiver* receiver_;
    int signal_id_;
    PyObject* callable_;
  };
  QList<SignalConnection> signal_connections_;
};

#endif // PYTHONSCRIPT_H
