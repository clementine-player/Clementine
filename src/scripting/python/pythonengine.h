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

#ifndef PYTHONENGINE_H
#define PYTHONENGINE_H

#include "scripting/languageengine.h"

struct _object; // PyObject
struct _sipAPIDef;
struct _sipTypeDef;

class PythonEngine : public LanguageEngine {
public:
  PythonEngine(ScriptManager* manager);

  ScriptManager::Language language() const { return ScriptManager::Language_Python; }
  QString name() const { return "python"; }

  Script* CreateScript(const QString& path, const QString& script_file,
                       const QString& id);

  const _sipAPIDef* sip_api() const { return sip_api_; }

  void AddLogLine(const QString& message, bool error = false);

private:
  static const _sipAPIDef* GetSIPApi();
  void AddObject(void* object, const _sipTypeDef* type, const char* name) const;

private:
  bool initialised_;

  _object* clementine_module_;
  const _sipAPIDef* sip_api_;
};

#endif // PYTHONENGINE_H
