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

struct _object; // PyObject
struct _sipAPIDef;
struct _sipTypeDef;
struct _ts; // PyThreadState

class PythonEngine;

class PythonScript : public Script {
public:
  PythonScript(PythonEngine* engine,
               const QString& path, const QString& script_file);

  bool Init();
  bool Unload();

private:
  void AddObject(void* object, const _sipTypeDef* type, const char* name) const;

private:
  PythonEngine* engine_;

  _ts* interpreter_;
  _object* clementine_module_;
  const _sipAPIDef* sip_api_;
};

#endif // PYTHONSCRIPT_H
