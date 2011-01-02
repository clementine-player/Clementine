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
#include <sip.h>

#include "pythonengine.h"
#include "pythonscript.h"
#include "sipAPIclementine.h"

#include <QFile>
#include <QtDebug>


PythonScript::PythonScript(PythonEngine* engine, const QString& path,
                           const QString& script_file, const QString& id)
  : Script(engine, path, script_file, id),
    engine_(engine)
{
}

bool PythonScript::Init() {
  engine_->AddLogLine("Loading script file \"" + script_file() + "\"", false);

  // Open the file
  QFile file(script_file());
  if (!file.open(QIODevice::ReadOnly)) {
    engine_->AddLogLine("Could not open file", true);
    return false;
  }

  PyEval_AcquireLock();

  // Create a module for this script
  // TODO: allowed characters?
  PyObject* module = PyImport_AddModule(id().toAscii().constData());
  PyObject* dict = PyModule_GetDict(module);

  // Add __builtins__
  PyObject* builtin_mod = PyImport_ImportModule("__builtin__");
  PyModule_AddObject(module, "__builtins__", builtin_mod);
  Py_DECREF(builtin_mod);

  // Set __file__
  PyModule_AddStringConstant(module, "__file__", script_file().toLocal8Bit().constData());

  // Set script
  PyObject* script = engine_->sip_api()->api_convert_from_type(
        interface(), sipType_ScriptInterface, NULL);
  PyModule_AddObject(module, "script", script);

  // Get a file stream from the file handle
  FILE* stream = fdopen(file.handle(), "r");

  // Run the script
  PyObject* result = PyRun_File(
      stream, script_file().toLocal8Bit().constData(), Py_file_input, dict, dict);
  if (result == NULL) {
    engine_->AddLogLine("Could not execute file", true);
    PyErr_Print();
    PyEval_ReleaseLock();
    return false;
  }

  Py_DECREF(result);
  PyEval_ReleaseLock();
  return true;
}



bool PythonScript::Unload() {
  // TODO: Actually do some cleanup
  return true;
}
