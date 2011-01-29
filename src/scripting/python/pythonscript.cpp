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
#include "scripting/scriptinfo.h"

#include <QFile>
#include <QtDebug>


PythonScript::PythonScript(PythonEngine* engine, const ScriptInfo& info)
  : Script(engine, info),
    engine_(engine),
    module_name_(QString(PythonEngine::kModulePrefix) + "." + info.id())
{
}

bool PythonScript::Init() {
  engine_->AddLogLine("Loading script file \"" + info().script_file() + "\"", false);

  // Open the file
  QFile file(info().script_file());
  if (!file.open(QIODevice::ReadOnly)) {
    engine_->AddLogLine("Could not open file", true);
    return false;
  }

  PyEval_AcquireLock();

  // Create a module for this script
  // TODO: allowed characters?
  PyObject* module = PyImport_AddModule(module_name_.toAscii().constData());
  PyObject* dict = PyModule_GetDict(module);

  // Add __builtins__
  PyObject* builtin_mod = PyImport_ImportModule("__builtin__");
  PyModule_AddObject(module, "__builtins__", builtin_mod);

  // Set __file__
  PyModule_AddStringConstant(module, "__file__", info().script_file().toLocal8Bit().constData());

  // Set __path__
  PyObject* __path__ = PyList_New(1);
  PyList_SetItem(__path__, 0, PyString_FromString(info().path().toLocal8Bit().constData()));
  PyModule_AddObject(module, "__path__", __path__);

  // Set script
  PyObject* script = engine_->sip_api()->api_convert_from_type(
        interface(), sipType_ScriptInterface, NULL);
  PyModule_AddObject(module, "script", script);

  // Get a file stream from the file handle
  FILE* stream = fdopen(file.handle(), "r");

  // Run the script
  PyObject* result = PyRun_File(stream,
      info().script_file().toLocal8Bit().constData(), Py_file_input, dict, dict);
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
  // Remove this module and all its children from sys.modules.  That should be
  // the only place that references it, so this will clean up the modules'
  // dict and all globals.
  PyEval_AcquireLock();
  PyInterpreterState *interp = PyThreadState_GET()->interp;
  PyObject* modules = interp->modules;

  QStringList keys_to_delete;

  Py_ssize_t pos = 0;
  PyObject* key;
  PyObject* value;
  while (PyDict_Next(modules, &pos, &key, &value)) {
    const char* name = PyString_AS_STRING(key);
    if (PyString_Check(key) && PyModule_Check(value)) {
      if (QString(name).startsWith(module_name_)) {
        keys_to_delete << name;
      }
    }
  }

  foreach (const QString& key, keys_to_delete) {
    // Workaround Python issue 10068 (only affects 2.7.0)
    _PyModule_Clear(PyDict_GetItemString(modules, key.toAscii().constData()));

    PyDict_DelItemString(modules, key.toAscii().constData());
  }
  PyEval_ReleaseLock();

  // Delete any native objects this script created
  qDeleteAll(native_objects_);
  native_objects_.clear();

  return true;
}
