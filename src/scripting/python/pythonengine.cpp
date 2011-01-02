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

const char* PythonEngine::kModulePrefix = "clementinescripts";

extern "C" {
  void initclementine();
}

PythonEngine::PythonEngine(ScriptManager* manager)
  : LanguageEngine(manager),
    initialised_(false)
{
}

const sipAPIDef* PythonEngine::GetSIPApi() {
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

Script* PythonEngine::CreateScript(const QString& path,
                                   const QString& script_file,
                                   const QString& id) {
  // Initialise Python if it hasn't been done yet
  if (!initialised_) {
    AddLogLine("Initialising python...", false);

    // Add the Clementine builtin module
    PyImport_AppendInittab(const_cast<char*>("clementine"), initclementine);

    // Initialise python
    Py_SetProgramName(const_cast<char*>("clementine"));
    PyEval_InitThreads();
    Py_InitializeEx(0);

    // Get the clementine module so we can put stuff in it
    clementine_module_ = PyImport_ImportModule("clementine");
    if (!clementine_module_) {
      AddLogLine("Failed to import the clementine module", true);
      if (PyErr_Occurred()) {
        PyErr_Print();
      }
      Py_Finalize();
      return NULL;
    }
    sip_api_ = GetSIPApi();

    // Add objects to the module
    AddObject(manager()->data().player_, sipType_Player, "player");
    AddObject(manager()->data().playlists_, sipType_PlaylistManager, "playlists");
    AddObject(this, sipType_PythonEngine, "pythonengine");

    // Create a module for scripts
    PyImport_AddModule(kModulePrefix);

    // Run the startup script - this redirects sys.stdout and sys.stderr to our
    // log handler.
    QFile python_startup(":pythonstartup.py");
    python_startup.open(QIODevice::ReadOnly);
    QByteArray python_startup_script = python_startup.readAll();

    if (PyRun_SimpleString(python_startup_script.constData()) != 0) {
      AddLogLine("Could not execute startup code", true);
      Py_Finalize();
      return NULL;
    }

    PyEval_ReleaseLock();

    initialised_ = true;
  }

  Script* ret = new PythonScript(this, path, script_file, id);
  if (ret->Init()) {
    return ret;
  }

  ret->Unload();
  delete ret;
  return NULL;
}

void PythonEngine::AddObject(void* object, const _sipTypeDef* type,
                             const char * name) const {
  PyObject* python_object = sip_api_->api_convert_from_type(object, type, NULL);
  PyModule_AddObject(clementine_module_, name, python_object);
}

void PythonEngine::AddLogLine(const QString& message, bool error) {
  manager()->AddLogLine("Python", message, error);
}
