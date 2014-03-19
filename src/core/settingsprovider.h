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

#ifndef SETTINGSPROVIDER_H
#define SETTINGSPROVIDER_H

#include <QVariant>
#include <QSettings>

class SettingsProvider {
 public:
  SettingsProvider();
  virtual ~SettingsProvider() {}

  virtual void set_group(const char* group) = 0;

  virtual QVariant value(const QString& key,
                         const QVariant& default_value = QVariant()) const = 0;
  virtual void setValue(const QString& key, const QVariant& value) = 0;
  virtual int beginReadArray(const QString& prefix) = 0;
  virtual void beginWriteArray(const QString& prefix, int size = -1) = 0;
  virtual void setArrayIndex(int i) = 0;
  virtual void endArray() = 0;
};

class DefaultSettingsProvider : public SettingsProvider {
 public:
  DefaultSettingsProvider();

  void set_group(const char* group);

  QVariant value(const QString& key,
                 const QVariant& default_value = QVariant()) const;
  void setValue(const QString& key, const QVariant& value);
  int beginReadArray(const QString& prefix);
  void beginWriteArray(const QString& prefix, int size = -1);
  void setArrayIndex(int i);
  void endArray();

 private:
  QSettings backend_;
};

#endif  // SETTINGSPROVIDER_H
