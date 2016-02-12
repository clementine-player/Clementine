/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>
   Copyright 2015 - 2016, Arun Narayanankutty <n.arun.lifescience@gmail.com>

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

#ifndef DBUSIDLEHANDLER_H
#define DBUSIDLEHANDLER_H

#include "idlehandler.h"

#include <QString>

class DBusIdleHandler : public IdleHandler {
 public:
  DBusIdleHandler(const QString& service, const QString& path,
                  const QString& interface);

  void Inhibit(const char* reason) override;
  void Uninhibit() override;
  bool Isinhibited() override;

 private:
  enum GnomeIdleHandlerFlags {
    Inhibit_Suspend = 4,
    Inhibit_Mark_Idle = 8
  };

  QString service_;
  QString path_;
  QString interface_;

  quint32 cookie_;
};

#endif  // DBUSIDLEHANDLER_H
