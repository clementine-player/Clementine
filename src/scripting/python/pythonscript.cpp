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

#include <Python.h>

#include "pythonscript.h"

#include <QFile>
#include <QtDebug>


PythonScript::PythonScript(const QString& path, const QString& script_file)
  : Script(path, script_file),
    interpreter_(NULL)
{
}

bool PythonScript::Init() {
  // Open the file
  QFile file(script_file());
  if (!file.open(QIODevice::ReadOnly)) {
    qWarning() << "Error opening file:" << script_file();
    return false;
  }

  // Create a python interpreter
  PyEval_AcquireLock();
  interpreter_ = Py_NewInterpreter();
  PyEval_ReleaseLock();

  // Get a file stream from the file handle
  FILE* stream = fdopen(file.handle(), "r");

  if (PyRun_SimpleFile(stream, script_file().toLocal8Bit().constData()) != 0) {
    PyEval_AcquireLock();
    Py_EndInterpreter(interpreter_);
    PyEval_ReleaseLock();

    interpreter_ = NULL;
    return false;
  }

  return true;
}

bool PythonScript::Unload() {
  PyEval_AcquireLock();
  Py_EndInterpreter(interpreter_);
  PyEval_ReleaseLock();

  interpreter_ = NULL;
  return true;
}
