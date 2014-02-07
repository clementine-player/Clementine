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

#ifndef QUERYPLAYLISTGENERATOR_H
#define QUERYPLAYLISTGENERATOR_H

#include "generator.h"
#include "search.h"

namespace smart_playlists {

class QueryGenerator : public Generator {
 public:
  QueryGenerator();
  QueryGenerator(const QString& name, const Search& search,
                 bool dynamic = false);

  QString type() const { return "Query"; }

  void Load(const Search& search);
  void Load(const QByteArray& data);
  QByteArray Save() const;

  PlaylistItemList Generate();
  PlaylistItemList GenerateMore(int count);
  bool is_dynamic() const { return dynamic_; }
  void set_dynamic(bool dynamic) { dynamic_ = dynamic; }

  Search search() const { return search_; }
  int GetDynamicFuture() { return search_.limit_; }

 private:
  Search search_;
  bool dynamic_;

  QList<int> previous_ids_;
  int current_pos_;
};

}  // namespace

#endif  // QUERYPLAYLISTGENERATOR_H
