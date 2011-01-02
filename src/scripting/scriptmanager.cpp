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
#include "scriptinterface.h"
#include "scriptmanager.h"
#include "core/utilities.h"

#ifdef HAVE_SCRIPTING_PYTHON
# include "scripting/python/pythonengine.h"
#endif

#include <QDirIterator>
#include <QSettings>
#include <QTextDocument>
#include <QtDebug>

const char* ScriptManager::kSettingsGroup = "Scripts";
const char* ScriptManager::kIniFileName = "script.ini";
const char* ScriptManager::kIniSettingsGroup = "Script";

ScriptManager::ScriptManager(QObject* parent)
  : QAbstractListModel(parent)
{
#ifdef HAVE_SCRIPTING_PYTHON
  engines_ << new PythonEngine(this);
#endif

  search_paths_ << Utilities::GetConfigPath(Utilities::Path_Scripts);
}

ScriptManager::~ScriptManager() {
  foreach (const ScriptInfo& info, info_) {
    if (info.loaded_) {
      info.loaded_->Unload();
      delete info.loaded_;
    }
  }

  qDeleteAll(engines_);
}

void ScriptManager::Init(const GlobalData& data) {
  data_ = data;

  // Load settings
  LoadSettings();

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

void ScriptManager::LoadSettings() {
  QSettings s;
  s.beginGroup(kSettingsGroup);
  enabled_scripts_ = QSet<QString>::fromList(
      s.value("enabled_scripts").toStringList());
}

void ScriptManager::SaveSettings() const {
  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.setValue("enabled_scripts", QVariant::fromValue<QStringList>(enabled_scripts_.toList()));
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
  const QString id = QFileInfo(path).completeBaseName();

  // Does the file exist?
  ScriptManager::ScriptInfo ret;
  if (!QFile::exists(ini_file)) {
    qWarning() << "Script definition file not found:" << ini_file;
    return ret;
  }

  // Open it
  QSettings s(ini_file, QSettings::IniFormat);
  if (!s.childGroups().contains(kIniSettingsGroup)) {
    qWarning() << "Missing" << kIniSettingsGroup << "section in" << ini_file;
    return ret;
  }
  s.beginGroup(kIniSettingsGroup);

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
  ret.id_ = id;
  ret.name_ = s.value("name").toString();
  ret.description_ = s.value("description").toString();
  ret.author_ = s.value("author").toString();
  ret.url_ = s.value("url").toString();
  ret.script_file_ = QFileInfo(QDir(path), s.value("script_file").toString()).absoluteFilePath();
  ret.icon_ = QIcon(QFileInfo(QDir(path), s.value("icon").toString()).absoluteFilePath());

  if (enabled_scripts_.contains(id)) {
    // Load the script if it's enabled
    ret.loaded_ = engine->CreateScript(path, ret.script_file_, ret.id_);
    if (!ret.loaded_) {
      // Failed to load?  Disable it so we don't try again
      enabled_scripts_.remove(id);
      SaveSettings();
    }
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

    case Qt::DecorationRole:
      return info.icon_;

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

    case Role_IsEnabled:
      return info.loaded_ != NULL;

    default:
      return QVariant();
  }
}

void ScriptManager::Enable(const QModelIndex& index) {
  if (index.row() < 0 || index.row() >= info_.count())
    return;

  ScriptInfo* info = &info_[index.row()];
  if (info->loaded_)
    return;

  // Find an engine for it
  LanguageEngine* engine = EngineForLanguage(info->language_);
  if (!engine) {
    qWarning() << "Unknown language in" << info->path_;
    return;
  }

  // Load the script
  info->loaded_ = engine->CreateScript(info->path_, info->script_file_, info->id_);

  // If it loaded correctly then automatically load it in the future
  if (info->loaded_) {
    enabled_scripts_.insert(info->id_);
    SaveSettings();
  }

  emit dataChanged(index, index);
}

void ScriptManager::Disable(const QModelIndex& index) {
  if (index.row() < 0 || index.row() >= info_.count())
    return;

  ScriptInfo* info = &info_[index.row()];
  if (!info->loaded_)
    return;

  info->loaded_->Unload();
  delete info->loaded_;
  info->loaded_ = NULL;

  enabled_scripts_.remove(info->id_);
  SaveSettings();

  emit dataChanged(index, index);
}

void ScriptManager::ShowSettingsDialog(const QModelIndex& index) {
  if (index.row() < 0 || index.row() >= info_.count())
    return;

  ScriptInfo* info = &info_[index.row()];
  if (!info->loaded_)
    return;

  info->loaded_->interface()->ShowSettingsDialog();
}

void ScriptManager::AddLogLine(const QString& who, const QString& message, bool error) {
  foreach (const QString& line, message.split("\n", QString::SkipEmptyParts)) {
    QString plain = "[" + who + "] " + line;
    QString html = "<b>[" + Qt::escape(who) + "]</b> " + Qt::escape(line);
    html.replace(' ', "&nbsp;");

    if (error) {
      html = "<font color=\"red\">" + html + "</font>";
    }

    log_lines_ << html;
    emit LogLineAdded(html);

    qDebug() << plain.toLocal8Bit().constData();
  }
}
