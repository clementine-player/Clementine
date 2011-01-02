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

#include <QAbstractItemModel>
#include <QIcon>
#include <QSet>
#include <QStringList>

class LanguageEngine;
class Player;
class PlaylistManager;
class Script;

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

  enum Language {
    Language_Unknown = 0,
    Language_Python,
  };

  struct GlobalData {
    GlobalData() {}
    GlobalData(Player* player, PlaylistManager* playlists)
      : player_(player),
        playlists_(playlists)
    {}

    Player* player_;
    PlaylistManager* playlists_;
  };

  static const char* kSettingsGroup;
  static const char* kIniFileName;
  static const char* kIniSettingsGroup;

  void Init(const GlobalData& data);
  const GlobalData& data() const { return data_; }

  void Enable(const QModelIndex& index);
  void Disable(const QModelIndex& index);
  void ShowSettingsDialog(const QModelIndex& index);

  QStringList log_lines() const { return log_lines_; }

  // QAbstractListModel
  int rowCount(const QModelIndex& parent = QModelIndex()) const;
  QVariant data(const QModelIndex& index, int role) const;

public slots:
  void AddLogLine(const QString& who, const QString& message, bool error);

signals:
  void LogLineAdded(const QString& html);

private:
  struct ScriptInfo {
    ScriptInfo() : language_(Language_Unknown), loaded_(NULL) {}

    bool is_valid() const { return language_ != Language_Unknown; }

    QString path_;
    QString id_;

    QString name_;
    QString description_;
    QString author_;
    QString url_;
    QIcon icon_;

    Language language_;
    QString script_file_;

    Script* loaded_;
  };

  void LoadSettings();
  void SaveSettings() const;

  ScriptInfo LoadScriptInfo(const QString& path);

  LanguageEngine* EngineForLanguage(const QString& language_name) const;
  LanguageEngine* EngineForLanguage(Language language) const;

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

  // Things available to scripts
  GlobalData data_;
};

#endif // SCRIPTMANAGER_H
