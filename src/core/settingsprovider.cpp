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

#include "settingsprovider.h"

SettingsProvider::SettingsProvider() {}

DefaultSettingsProvider::DefaultSettingsProvider() {}

void DefaultSettingsProvider::set_group(const char* group) {
  while (!backend_.group().isEmpty()) backend_.endGroup();

  backend_.beginGroup(group);
}

QVariant DefaultSettingsProvider::value(const QString& key,
                                        const QVariant& default_value) const {
  return backend_.value(key, default_value);
}

void DefaultSettingsProvider::setValue(const QString& key,
                                       const QVariant& value) {
  backend_.setValue(key, value);
}

int DefaultSettingsProvider::beginReadArray(const QString& prefix) {
  return backend_.beginReadArray(prefix);
}

void DefaultSettingsProvider::beginWriteArray(const QString& prefix, int size) {
  backend_.beginWriteArray(prefix, size);
}

void DefaultSettingsProvider::setArrayIndex(int i) {
  backend_.setArrayIndex(i);
}

void DefaultSettingsProvider::endArray() { backend_.endArray(); }
