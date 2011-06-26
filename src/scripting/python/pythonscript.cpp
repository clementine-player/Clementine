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
#include <PythonQtSignalReceiver.h>

#include "pythonengine.h"
#include "pythonscript.h"
#include "core/logging.h"
#include "scripting/scriptinfo.h"
#include "scripting/scriptinterface.h"

#include <QFile>
#include <QtDebug>


PythonScript::PythonScript(PythonEngine* engine, const ScriptInfo& info)
  : Script(engine, info),
    engine_(engine),
    module_name_(QString(PythonEngine::kScriptModulePrefix) + "." + info.id())
{
}

bool PythonScript::Init() {
  engine_->manager()->AddLogLine("Python",
      "Loading script file \"" + info().script_file() + "\"", false);

  PythonQt* python_qt = PythonQt::self();

  // Create a module for the script
  module_ = python_qt->createModuleFromScript(module_name_);

  // Set __path__ - don't use PythonQtObjectPtr::addVariable because it sets
  // it as a tuple instead of a list.
  PyObject* __path__ = PyList_New(1);
  PyList_SetItem(__path__, 0, PyString_FromString(info().path().toLocal8Bit().constData()));
  PyModule_AddObject(module_, "__path__", __path__);

  // Set __file__
  module_.addVariable("__file__", info().script_file());

  // Set script object
  module_.addObject("__script__", interface());

  // Eval the script
  PythonQtObjectPtr code = python_qt->parseFile(info().script_file());
  if (code) {
    PyObject* dict = PyModule_GetDict(module_);
    PyObject* r = PyEval_EvalCode((PyCodeObject*)code.object(), dict, dict);
    if (r) {
      Py_DECREF(r);
    }
  }

  if (PyErr_Occurred()) {
    PythonQt::self()->handleError();
    engine_->manager()->AddLogLine("Python", "Failed to create module for script", true);
    return false;
  }

  return true;
}

bool PythonScript::Unload() {
  // Disconnect any signal connections that this script made while it was
  // running.  This is important because those connections will hold references
  // to bound methods in the script's classes, so the classes won't get deleted.
  foreach (const SignalConnection& conn, signal_connections_) {
    conn.receiver_->removeSignalHandler(conn.signal_id_, conn.callable_);
  }

  // Remove this module and all its children from sys.modules.  That should be
  // the only place that references it, so this will clean up the modules'
  // dict and all globals.
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

  module_ = PythonQtObjectPtr();

  PyGC_Collect();

  return true;
}

void PythonScript::RegisterSignalConnection(PythonQtSignalReceiver* receiver,
                                            int signal_id, PyObject* callable) {
  signal_connections_ << SignalConnection(receiver, signal_id, callable);
}
