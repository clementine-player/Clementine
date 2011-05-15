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

#include <PythonQt.h>

#include "scripting/languageengine.h"

#include "gtest/gtest_prod.h"

class PythonEngine : public LanguageEngine {
  Q_OBJECT

  FRIEND_TEST(PythonTest, SharedPointers);

public:
  PythonEngine(ScriptManager* manager);
  ~PythonEngine();

  static PythonEngine* instance() { return sInstance; }

  static const char* kClementineModuleName;
  static const char* kScriptModulePrefix;

  ScriptInfo::Language language() const { return ScriptInfo::Language_Python; }
  QString name() const { return "python"; }

  bool EnsureInitialised();

  Script* CreateScript(const ScriptInfo& info);
  void DestroyScript(Script* script);

private slots:
  void PythonStdOut(const QString& str);
  void PythonStdErr(const QString& str);

private:
  static PythonEngine* sInstance;
  bool initialised_;

  PythonQtObjectPtr clementine_module_;
  PythonQtObjectPtr scripts_module_;

  QMap<QString, Script*> loaded_scripts_;
};

#endif // PYTHONENGINE_H
