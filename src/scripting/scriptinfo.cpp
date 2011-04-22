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

#include "languageengine.h"
#include "scriptinfo.h"
#include "core/logging.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QtDebug>

const char* ScriptInfo::kIniFileName = "script.ini";
const char* ScriptInfo::kIniSettingsGroup = "Script";

ScriptInfo::ScriptInfo()
  : d(new Private)
{
}

void ScriptInfo::InitFromDirectory(const ScriptManager* manager, const QString& path) {
  const QString ini_file = path + "/" + kIniFileName;
  const QString id = QFileInfo(path).fileName();

  // Does the file exist?
  if (!QFile::exists(ini_file)) {
    qLog(Warning) << "Script definition file not found:" << ini_file;
    return;
  }

  InitFromFile(manager, id, path, ini_file);
}

void ScriptInfo::InitFromFile(const ScriptManager* manager,
                              const QString& id,
                              const QString& path,
                              const QString& ini_file) {
  // Open it
  QSettings s(ini_file, QSettings::IniFormat);
  if (!s.childGroups().contains(kIniSettingsGroup)) {
    qLog(Warning) << "Missing" << kIniSettingsGroup << "section in" << ini_file;
    return;
  }
  s.beginGroup(kIniSettingsGroup);

  // Find out what language it's in
  QString language_name = s.value("language").toString();
  LanguageEngine* engine = manager->EngineForLanguage(language_name);
  if (!engine) {
    qLog(Warning) << "Unknown language" << language_name << "in" << ini_file;
    return;
  }
  d->language_ = engine->language();

  // Load the rest of the metadata
  d->path_ = path;
  d->id_ = id;
  d->name_ = s.value("name").toString();
  d->description_ = s.value("description").toString();
  d->author_ = s.value("author").toString();
  d->url_ = s.value("url").toString();
  d->script_file_ = QFileInfo(QDir(path), s.value("script_file").toString()).absoluteFilePath();
  d->icon_filename_ = QFileInfo(QDir(path), s.value("icon").toString()).absoluteFilePath();
}

bool ScriptInfo::operator ==(const ScriptInfo& other) const {
  return path() == other.path() &&
         name() == other.name() &&
         description() == other.description() &&
         author() == other.author() &&
         url() == other.url() &&
         language() == other.language() &&
         script_file() == other.script_file();
}

bool ScriptInfo::operator !=(const ScriptInfo& other) const {
  return !(*this == other);
}

void ScriptInfo::TakeMetadataFrom(const ScriptInfo& other) {
  d->path_ = other.path();
  d->name_ = other.name();
  d->description_ = other.description();
  d->author_ = other.author();
  d->url_ = other.url();
  d->icon_filename_ = other.icon_filename();
  d->language_ = other.language();
  d->script_file_ = other.script_file();
}
