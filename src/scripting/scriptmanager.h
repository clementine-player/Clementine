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
    GlobalData() : player_(NULL) {}
    GlobalData(Player* player) : player_(player) {}

    Player* player_;
  };

  static const char* kSettingsGroup;
  static const char* kIniFileName;
  static const char* kIniSettingsGroup;

  void Init(const GlobalData& data);

  void Enable(const QModelIndex& index);
  void Disable(const QModelIndex& index);
  void ShowSettingsDialog(const QModelIndex& index);

  // QAbstractListModel
  int rowCount(const QModelIndex& parent = QModelIndex()) const;
  QVariant data(const QModelIndex& index, int role) const;

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
  QList<LanguageEngine*> engines_;

  QStringList search_paths_;
  QList<ScriptInfo> info_;

  QSet<QString> enabled_scripts_;

  Player* player_;
};

#endif // SCRIPTMANAGER_H
