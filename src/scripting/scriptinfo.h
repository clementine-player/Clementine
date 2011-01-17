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

#ifndef SCRIPTINFO_H
#define SCRIPTINFO_H

#include <QIcon>
#include <QSharedData>
#include <QSharedDataPointer>

class Script;
class ScriptManager;

class ScriptInfo {
public:
  enum Language {
    Language_Unknown = 0,
    Language_Python,
  };

  static const char* kIniFileName;
  static const char* kIniSettingsGroup;

  void InitFromDirectory(const ScriptManager* manager, const QString& path);
  void InitFromFile(const ScriptManager* manager,
                    const QString& id,
                    const QString& path,
                    const QString& filename);
  void TakeMetadataFrom(const ScriptInfo& other);

  bool is_valid() const { return d->language_ != Language_Unknown; }
  bool operator ==(const ScriptInfo& other) const;
  bool operator !=(const ScriptInfo& other) const;

  const QString& path() const { return d->path_; }
  const QString& id() const { return d->id_; }

  const QString& name() const { return d->name_; }
  const QString& description() const { return d->description_; }
  const QString& author() const { return d->author_; }
  const QString& url() const { return d->url_; }
  const QIcon& icon() const { return d->icon_; }

  Language language() const { return d->language_; }
  const QString& script_file() const { return d->script_file_; }

  Script* loaded() const { return d->loaded_; }
  void set_loaded(Script* loaded) { d->loaded_ = loaded; }

private:
  struct Private : public QSharedData {
    Private() : language_(Language_Unknown), loaded_(NULL) {}

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

  QSharedDataPointer<Private> d;
};

#endif // SCRIPTINFO_H
