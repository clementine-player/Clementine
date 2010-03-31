/* This file is part of Clementine.

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

#include "resources_env.h"
#include "metatypes_env.h"

#ifdef WIN32
# include <QtPlugin>
  Q_IMPORT_PLUGIN(qsqlite)
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

  return RUN_ALL_TESTS();
}
