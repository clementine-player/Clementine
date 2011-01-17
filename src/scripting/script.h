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

#ifndef SCRIPT_H
#define SCRIPT_H

#include "scriptinfo.h"

#include <QList>
#include <QMetaType>
#include <QString>

#include <boost/scoped_ptr.hpp>

class LanguageEngine;
class ScriptInterface;

class QObject;

class Script {
public:
  Script(LanguageEngine* language, const ScriptInfo& info);
  virtual ~Script();

  LanguageEngine* language() const { return language_; }
  const ScriptInfo& info() const { return info_; }
  ScriptInterface* interface() const { return interface_.get(); }

  // The script can "own" QObjects like QActions that must be deleted (and
  // removed from the UI, etc.) when the script is unloaded.
  void AddNativeObject(QObject* object);
  void RemoveNativeObject(QObject* object);

  virtual bool Init() = 0;
  virtual bool Unload() = 0;

protected:
  QList<QObject*> native_objects_;

private:
  Q_DISABLE_COPY(Script);

  boost::scoped_ptr<ScriptInterface> interface_;
  LanguageEngine* language_;
  ScriptInfo info_;
};
Q_DECLARE_METATYPE(Script*);

#endif // SCRIPT_H
