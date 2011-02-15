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
  Q_OBJECT

public:
  PythonEngine(ScriptManager* manager);
  ~PythonEngine();

  static PythonEngine* instance() { return sInstance; }

  static const char* kModulePrefix;

  ScriptInfo::Language language() const { return ScriptInfo::Language_Python; }
  QString name() const { return "python"; }

  bool EnsureInitialised();

  Script* CreateScript(const ScriptInfo& info);
  void DestroyScript(Script* script);

  const _sipAPIDef* sip_api() const { return sip_api_; }

  void AddLogLine(const QString& message, bool error = false);
  void RegisterNativeObject(QObject* object);

private:
  static const _sipAPIDef* GetSIPApi();
  void AddObject(void* object, const _sipTypeDef* type, const char* name) const;

  // Looks for a loaded script whose ID either exactly matches id, or matches
  // some string at the start of id followed by a dot.  For example,
  // FindScriptMatchingId("foo") and FindScriptMatchingId("foo.bar") would both
  // match a Script with an ID of foo, but FindScriptMatchingId("foobar")
  // would not.
  Script* FindScriptMatchingId(const QString& id) const;

private slots:
  void NativeObjectDestroyed(QObject* object);

private:
  static PythonEngine* sInstance;

  bool initialised_;
  _object* clementine_module_;
  const _sipAPIDef* sip_api_;

  QMap<QString, Script*> loaded_scripts_;
};

#endif // PYTHONENGINE_H
