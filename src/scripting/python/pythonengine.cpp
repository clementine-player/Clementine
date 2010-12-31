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

extern "C" {
  void initclementine();
}

PythonEngine::PythonEngine()
  : initialised_(false)
{
}

void PythonEngine::Init(const ScriptManager::GlobalData& data) {
  data_ = data;
}

Script* PythonEngine::CreateScript(const QString& path, const QString& script_file) {
  // Initialise Python if it hasn't been done yet
  if (!initialised_) {
    // Add the Clementine builtin module
    PyImport_AppendInittab(const_cast<char*>("clementine"), initclementine);

    Py_SetProgramName(const_cast<char*>("clementine"));
    PyEval_InitThreads();
    Py_InitializeEx(0);
    PyEval_ReleaseLock();

    initialised_ = true;
  }

  return new PythonScript(this, path, script_file);
}
