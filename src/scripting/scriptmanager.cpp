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

#include "config.h"
#include "languageengine.h"
#include "script.h"
#include "scriptmanager.h"
#include "core/utilities.h"

#ifdef HAVE_SCRIPTING_PYTHON
# include "scripting/python/pythonengine.h"
#endif

#include <QDirIterator>
#include <QSettings>
#include <QtDebug>

const char* ScriptManager::kIniFileName = "script.ini";

ScriptManager::ScriptManager(QObject* parent)
  : QAbstractListModel(parent)
{
#ifdef HAVE_SCRIPTING_PYTHON
  engines_ << new PythonEngine;
#endif

  search_paths_ << Utilities::GetConfigPath(Utilities::Path_Scripts);

  Reset();
}

ScriptManager::~ScriptManager() {
  qDeleteAll(engines_);
}

void ScriptManager::Reset() {
  // Remove any scripts that aren't loaded
  for (int i=0 ; i<info_.count() ; ++i) {
    if (!info_[i].loaded_) {
      info_.removeAt(i);
      --i;
    }
  }

  // Search for scripts
  foreach (const QString& search_path, search_paths_) {
    QDirIterator it(search_path,
        QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable | QDir::Executable,
        QDirIterator::FollowSymlinks);
    while (it.hasNext()) {
      it.next();
      const QString path = it.filePath();
      ScriptInfo info = LoadScriptInfo(path);

      if (!info.is_valid()) {
        qWarning() << "Not a valid Clementine script directory, ignoring:"
                   << path;
        continue;
      }

      info_ << info;
    }
  }

  reset();
}

LanguageEngine* ScriptManager::EngineForLanguage(const QString& language_name) const {
  foreach (LanguageEngine* engine, engines_) {
    if (engine->name() == language_name) {
      return engine;
    }
  }
  return NULL;
}

LanguageEngine* ScriptManager::EngineForLanguage(ScriptManager::Language language) const {
  foreach (LanguageEngine* engine, engines_) {
    if (engine->language() == language) {
      return engine;
    }
  }
  return NULL;
}

ScriptManager::ScriptInfo ScriptManager::LoadScriptInfo(const QString& path) {
  const QString ini_file = path + "/" + kIniFileName;

  // Does the file exist?
  ScriptManager::ScriptInfo ret;
  if (!QFile::exists(ini_file)) {
    qWarning() << "Script definition file not found:" << ini_file;
    return ret;
  }

  // Open it
  QSettings s(ini_file, QSettings::IniFormat);
  if (!s.childGroups().contains("Script")) {
    qWarning() << "Missing [Script] section in" << ini_file;
    return ret;
  }
  s.beginGroup("Script");

  // Find out what language it's in
  QString language_name = s.value("language").toString();
  LanguageEngine* engine = EngineForLanguage(language_name);
  if (!engine) {
    qWarning() << "Unknown language" << language_name << "in" << ini_file;
    return ret;
  }
  ret.language_ = engine->language();

  // Load the rest of the metadata
  ret.path_ = path;
  ret.name_ = s.value("name").toString();
  ret.description_ = s.value("description").toString();
  ret.author_ = s.value("author").toString();
  ret.url_ = s.value("url").toString();
  ret.script_file_ = path + "/" + s.value("script_file").toString();

  // Load the script - TODO: move somewhere else
  ret.loaded_ = engine->CreateScript(path, ret.script_file_);
  if (ret.loaded_) {
    ret.loaded_->Init();
  }

  return ret;
}

int ScriptManager::rowCount(const QModelIndex& parent) const {
  if (parent.isValid())
    return 0;
  return info_.count();
}

QVariant ScriptManager::data(const QModelIndex& index, int role) const {
  if (index.parent().isValid() || index.row() < 0 || index.row() >= info_.count())
    return QVariant();

  const ScriptInfo& info = info_[index.row()];

  switch (role) {
    case Qt::DisplayRole:
      return info.name_;

    case Role_Author:
      return info.author_;

    case Role_Description:
      return info.description_;

    case Role_Language:
      return info.language_;

    case Role_ScriptFile:
      return info.script_file_;

    case Role_Url:
      return info.url_;

    default:
      return QVariant();
  }
}
