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

  // Set script object
  module_.addObject("script", interface());

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
  module_ = PythonQtObjectPtr();

  return true;
}
