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

#ifndef SMARTPLAYLISTSEARCH_H
#define SMARTPLAYLISTSEARCH_H

#include "smartplaylistsearchterm.h"

class SmartPlaylistSearch {
public:
  SmartPlaylistSearch();

  // These values are persisted, so add to the end of the enum only
  enum SortType {
    Sort_Random = 0,
    Sort_FieldAsc,
    Sort_FieldDesc,
  };

  bool is_valid() const { return !terms_.isEmpty(); }

  QList<SmartPlaylistSearchTerm> terms_;
  SortType sort_type_;
  SmartPlaylistSearchTerm::Field sort_field_;
  int limit_;

  void Reset();
  QString ToSql(const QString& songs_table) const;
};

QDataStream& operator <<(QDataStream& s, const SmartPlaylistSearch& search);
QDataStream& operator >>(QDataStream& s, SmartPlaylistSearch& search);

#endif // SMARTPLAYLISTSEARCH_H
