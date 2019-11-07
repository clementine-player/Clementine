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

#include <gmock/gmock.h>

#include <QApplication>

#include "logging_env.h"
#include "metatypes_env.h"
#include "resources_env.h"

#if defined(Q_OS_WIN32) || defined(Q_OS_DARWIN)
# include <QtPlugin>
  Q_IMPORT_PLUGIN(QSQLiteDriverPlugin)
#endif

int main(int argc, char** argv) {
  testing::InitGoogleMock(&argc, argv);

  testing::AddGlobalTestEnvironment(new MetatypesEnvironment);
  #ifdef GUI
  QApplication a(argc, argv);
  #else
  QCoreApplication a(argc, argv);
  #endif
  testing::AddGlobalTestEnvironment(new ResourcesEnvironment);
  testing::AddGlobalTestEnvironment(new LoggingEnvironment);

  return RUN_ALL_TESTS();
}
