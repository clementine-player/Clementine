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

#include <QApplication>
#include <QFile>

#include "playlist/playlistitem.h"
#include "scripting/scriptmanager.h"
#include "scripting/python/pythonengine.h"

int main(int argc, char** argv) {
  QApplication a(argc, argv);

  // Create the python engine
  ScriptManager manager;
  LanguageEngine* language_engine =
      manager.EngineForLanguage(ScriptInfo::Language_Python);
  PythonEngine* python_engine = qobject_cast<PythonEngine*>(language_engine);

  // Initialise python
  if (!python_engine->EnsureInitialised()) {
    qFatal("Failed to initialise Python engine");
  }

  // Load the python script
  QFile script(":/doc/python/generate_python_docs.py");
  script.open(QIODevice::ReadOnly);
  QByteArray script_data = script.readAll();

  // Run it
  PyEval_AcquireLock();
  if (PyRun_SimpleString(script_data.constData()) != 0) {
    qFatal("Could not execute generate_python_docs.py");
  }
  PyEval_ReleaseLock();

  return 0;
}
