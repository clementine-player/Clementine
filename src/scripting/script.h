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

#include <QString>

class Script {
public:
  Script(const QString& path, const QString& script_file);
  virtual ~Script() {}

  const QString& path() const { return path_; }
  const QString& script_file() const { return script_file_; }

  virtual bool Init() = 0;
  virtual bool Unload() = 0;
  virtual bool Reload() { return Unload() && Init(); }

private:
  QString path_;
  QString script_file_;
};

#endif // SCRIPT_H
