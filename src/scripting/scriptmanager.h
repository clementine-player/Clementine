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

#ifndef SCRIPTMANAGER_H
#define SCRIPTMANAGER_H

#include "scriptinfo.h"

#include <QAbstractItemModel>
#include <QIcon>
#include <QSet>
#include <QStringList>

class LanguageEngine;
class Library;
class LibraryView;
class PlayerInterface;
class PlaylistManager;
class RadioModel;
class Script;
class SettingsDialog;
class TaskManager;
class UIInterface;

class QFileSystemWatcher;

class ScriptManager : public QAbstractListModel {
  Q_OBJECT

public:
  ScriptManager(QObject* parent = 0);
  ~ScriptManager();

  enum Role {
    Role_Description = Qt::UserRole,
    Role_Author,
    Role_Url,
    Role_Language,
    Role_ScriptFile,
    Role_IsEnabled,

    RoleCount
  };

  struct GlobalData {
    GlobalData() : valid_(false) {}
    GlobalData(Library* library, LibraryView* library_view,
               PlayerInterface* player, PlaylistManager* playlists,
               TaskManager* task_manager, SettingsDialog* settings_dialog,
               RadioModel* radio_model)
      : valid_(true),
        library_(library),
        library_view_(library_view),
        player_(player),
        playlists_(playlists),
        task_manager_(task_manager),
        settings_dialog_(settings_dialog),
        radio_model_(radio_model)
    {}

    bool valid_;
    Library* library_;
    LibraryView* library_view_;
    PlayerInterface* player_;
    PlaylistManager* playlists_;
    TaskManager* task_manager_;
    SettingsDialog* settings_dialog_;
    RadioModel* radio_model_;
  };

  static const char* kSettingsGroup;

  void Init(const GlobalData& data);
  const GlobalData& data() const { return data_; }
  UIInterface* ui() const { return ui_interface_; }

  void Enable(const QModelIndex& index);
  void Disable(const QModelIndex& index);
  void ShowSettingsDialog(const QModelIndex& index);

  QStringList log_lines() const { return log_lines_; }
  QStringList log_lines_plain() const { return log_lines_plain_; }

  // QAbstractListModel
  int rowCount(const QModelIndex& parent = QModelIndex()) const;
  QVariant data(const QModelIndex& index, int role) const;

  // These need to be public for ScriptInfo
  LanguageEngine* EngineForLanguage(const QString& language_name) const;
  LanguageEngine* EngineForLanguage(ScriptInfo::Language language) const;

public slots:
  void AddLogLine(const QString& who, const QString& message, bool error);

signals:
  void LogLineAdded(const QString& html);

private:
  void LoadSettings();
  void SaveSettings() const;

  QMap<QString, ScriptInfo> LoadAllScriptInfo() const;

  void MaybeAutoEnable(ScriptInfo* info);

private slots:
  void ScriptDirectoryChanged();
  void RescanScripts();

private:
  // Language engines
  QList<LanguageEngine*> engines_;

  // All scripts we know about
  QStringList search_paths_;
  QList<ScriptInfo> info_;

  // Names of scripts that get loaded automatically
  QSet<QString> enabled_scripts_;

  // HTML log messages
  QStringList log_lines_;
  QStringList log_lines_plain_;

  // Things available to scripts
  GlobalData data_;
  UIInterface* ui_interface_;

  // Watches script directories for changes
  QFileSystemWatcher* watcher_;
  QTimer* rescan_timer_;
};

#endif // SCRIPTMANAGER_H
