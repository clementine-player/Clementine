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


static const sipAPIDef* GetSIPApi() {
#if defined(SIP_USE_PYCAPSULE)
  return (const sipAPIDef *)PyCapsule_Import("sip._C_API", 0);
#else
  PyObject *sip_module;
  PyObject *sip_module_dict;
  PyObject *c_api;

  /* Import the SIP module. */
  sip_module = PyImport_ImportModule("sip");

  if (sip_module == NULL)
      return NULL;

  /* Get the module's dictionary. */
  sip_module_dict = PyModule_GetDict(sip_module);

  /* Get the "_C_API" attribute. */
  c_api = PyDict_GetItemString(sip_module_dict, "_C_API");

  if (c_api == NULL)
      return NULL;

  /* Sanity check that it is the right type. */
  if (!PyCObject_Check(c_api))
      return NULL;

  /* Get the actual pointer from the object. */
  return (const sipAPIDef *)PyCObject_AsVoidPtr(c_api);
#endif
}


PythonScript::PythonScript(PythonEngine* engine,
                           const QString& path, const QString& script_file)
  : Script(path, script_file),
    engine_(engine),
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

  // Get the clementine module so we can put stuff in it
  PyObject* clementine = PyImport_ImportModule("clementine");

  const sipAPIDef* sip_api = GetSIPApi();
  PyObject* player = sip_api->api_convert_from_type(
      engine_->data().player_, sipType_Player, NULL);
  PyModule_AddObject(clementine, "player", player);

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
