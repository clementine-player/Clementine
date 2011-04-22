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
#include "uiinterface.h"
#include "core/logging.h"
#include "core/utilities.h"

#ifdef HAVE_SCRIPTING_PYTHON
# include "scripting/python/pythonengine.h"
#endif

#ifdef Q_OS_MAC
# include "core/mac_startup.h"
#endif

#include <QCoreApplication>
#include <QDirIterator>
#include <QFileSystemWatcher>
#include <QSettings>
#include <QTextDocument>
#include <QTimer>
#include <QtDebug>

const char* ScriptManager::kSettingsGroup = "Scripts";

ScriptManager::ScriptManager(QObject* parent)
  : QAbstractListModel(parent),
    ui_interface_(new UIInterface(this)),
    watcher_(new QFileSystemWatcher(this)),
    rescan_timer_(new QTimer(this))
{
#ifdef HAVE_SCRIPTING_PYTHON
  engines_ << new PythonEngine(this);
#endif

  connect(watcher_, SIGNAL(directoryChanged(QString)), SLOT(ScriptDirectoryChanged()));

  rescan_timer_->setSingleShot(true);
  rescan_timer_->setInterval(1000);
  connect(rescan_timer_, SIGNAL(timeout()), SLOT(RescanScripts()));

  // Create the user's scripts directory if it doesn't exist yet
  QString local_path = Utilities::GetConfigPath(Utilities::Path_Scripts);
  if (!QFile::exists(local_path)) {
    if (!QDir().mkpath(local_path)) {
      qLog(Warning) << "Couldn't create directory" << local_path;
    }
  }

  search_paths_
      << local_path
#ifdef USE_INSTALL_PREFIX
      << CMAKE_INSTALL_PREFIX "/share/clementine/scripts"
#endif
      << "/usr/share/clementine/scripts"
      << "/usr/local/share/clementine/scripts";

#if defined(Q_OS_WIN32)
  search_paths_ << QCoreApplication::applicationDirPath() + "/scripts";
#elif defined(Q_OS_MAC)
  search_paths_ << mac::GetResourcesPath() + "/scripts";
#endif
}

ScriptManager::~ScriptManager() {
  foreach (const ScriptInfo& info, info_) {
    if (info.loaded()) {
      info.loaded()->language()->DestroyScript(info.loaded());
    }
  }
  qDeleteAll(engines_);
}

void ScriptManager::Init(const GlobalData& data) {
  data_ = data;

  // Load settings
  LoadSettings();

  // Search for scripts
  info_ = LoadAllScriptInfo().values();

  // Enable the ones that were enabled last time
  for (int i=0 ; i<info_.count() ; ++i) {
    MaybeAutoEnable(&info_[i]);
  }

  reset();
}

void ScriptManager::MaybeAutoEnable(ScriptInfo* info) {
  // Load the script if it's enabled
  if (enabled_scripts_.contains(info->id())) {
    // Find an engine for it
    LanguageEngine* engine = EngineForLanguage(info->language());
    if (!engine) {
      qLog(Warning) << "Unknown language in" << info->path();
      return;
    }

    info->set_loaded(engine->CreateScript(*info));
    if (!info->loaded()) {
      // Failed to load?  Disable it so we don't try again
      enabled_scripts_.remove(info->id());
      SaveSettings();
    }
  }
}

QMap<QString, ScriptInfo> ScriptManager::LoadAllScriptInfo() const {
  QMap<QString, ScriptInfo> ret;

  foreach (const QString& search_path, search_paths_) {
    if (!QFile::exists(search_path))
      continue;

    if (!watcher_->directories().contains(search_path)) {
      watcher_->addPath(search_path);
    }

    QDirIterator it(search_path,
        QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable | QDir::Executable,
        QDirIterator::FollowSymlinks);
    while (it.hasNext()) {
      it.next();
      const QString path = it.filePath();
      if (!watcher_->directories().contains(path)) {
        watcher_->addPath(path);
      }

      ScriptInfo info;
      info.InitFromDirectory(this, path);
      if (!info.is_valid()) {
        qLog(Warning) << "Not a valid Clementine script directory, ignoring:"
                   << path;
        continue;
      }

      if (ret.contains(info.id())) {
        // Seen this script already
        continue;
      }

      ret.insert(info.id(), info);
    }
  }

  return ret;
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

LanguageEngine* ScriptManager::EngineForLanguage(ScriptInfo::Language language) const {
  foreach (LanguageEngine* engine, engines_) {
    if (engine->language() == language) {
      return engine;
    }
  }
  return NULL;
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
      return info.name();

    case Qt::DecorationRole:
      return info.icon();

    case Role_Author:
      return info.author();

    case Role_Description:
      return info.description();

    case Role_Language:
      return info.language();

    case Role_ScriptFile:
      return info.script_file();

    case Role_Url:
      return info.url();

    case Role_IsEnabled:
      return info.loaded() != NULL;

    default:
      return QVariant();
  }
}

void ScriptManager::Enable(const QModelIndex& index) {
  if (index.row() < 0 || index.row() >= info_.count())
    return;

  ScriptInfo* info = &info_[index.row()];
  if (info->loaded())
    return;

  // Find an engine for it
  LanguageEngine* engine = EngineForLanguage(info->language());
  if (!engine) {
    qLog(Warning) << "Unknown language in" << info->path();
    return;
  }

  // Load the script
  info->set_loaded(engine->CreateScript(*info));

  // If it loaded correctly then automatically load it in the future
  if (info->loaded()) {
    enabled_scripts_.insert(info->id());
    SaveSettings();
  }

  emit dataChanged(index, index);
}

void ScriptManager::Disable(const QModelIndex& index) {
  if (index.row() < 0 || index.row() >= info_.count())
    return;

  ScriptInfo* info = &info_[index.row()];
  if (!info->loaded())
    return;

  info->loaded()->language()->DestroyScript(info->loaded());
  info->set_loaded(NULL);

  enabled_scripts_.remove(info->id());
  SaveSettings();

  emit dataChanged(index, index);
}

void ScriptManager::ShowSettingsDialog(const QModelIndex& index) {
  if (index.row() < 0 || index.row() >= info_.count())
    return;

  ScriptInfo* info = &info_[index.row()];
  if (!info->loaded())
    return;

  info->loaded()->interface()->ShowSettingsDialog();
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
    log_lines_plain_ << plain;
    emit LogLineAdded(html);

    qLog(Info) << plain.toLocal8Bit().constData();
  }
}

void ScriptManager::ScriptDirectoryChanged() {
  rescan_timer_->start();
}

void ScriptManager::RescanScripts() {
  // Get the new list of scripts
  QMap<QString, ScriptInfo> new_info = LoadAllScriptInfo();

  // Look at existing scripts, find ones that have changed or been deleted
  for (int i=0 ; i<info_.count() ; ++i) {
    ScriptInfo* info = &info_[i];
    const QString id = info->id();

    if (!new_info.contains(id)) {
      // This script was deleted - unload it and remove it from the model
      Disable(index(i));

      beginRemoveRows(QModelIndex(), i, i);
      info_.removeAt(i);
      endRemoveRows();

      --i;
      continue;
    }

    if (*info != new_info[id]) {
      // This script was modified - change the metadata in the existing entry
      info->TakeMetadataFrom(new_info[id]);

      emit dataChanged(index(i), index(i));

      AddLogLine("Watcher",
        tr("The '%1' script was modified, you might have to reload it").arg(id), false);
    }

    new_info.remove(id);
  }

  // Things that are left in new_info are newly added scripts
  if (!new_info.isEmpty()) {
    const int begin = info_.count();
    const int end = begin + new_info.count() - 1;

    beginInsertRows(QModelIndex(), begin, end);
    info_.append(new_info.values());

    for (int i=begin ; i<=end ; ++i) {
      MaybeAutoEnable(&info_[i]);
    }

    endInsertRows();
  }
}
