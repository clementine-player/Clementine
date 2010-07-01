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

#include <sqlite3.h>

int main(int argc, char** argv) {
  sqlite3* db;

  int ret = sqlite3_open(":memory:", &db);
  if (ret)
    return 1;

  char* error = 0;
  ret = sqlite3_exec(db, "CREATE VIRTUAL TABLE foo USING fts3()", 0, 0, &error);
  if (ret != SQLITE_OK)
    return 1;

  return 0;
}
