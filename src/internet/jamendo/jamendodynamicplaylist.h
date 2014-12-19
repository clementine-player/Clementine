/* This file is part of Clementine.
   Copyright 2010, David Sansome <davidsansome@gmail.com>
   Copyright 2010, 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2011, Arnaud Bienner <arnaud.bienner@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>

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

#ifndef INTERNET_JAMENDO_JAMENDODYNAMICPLAYLIST_H_
#define INTERNET_JAMENDO_JAMENDODYNAMICPLAYLIST_H_

#include "smartplaylists/generator.h"

class JamendoDynamicPlaylist : public smart_playlists::Generator {
  Q_OBJECT
  friend QDataStream& operator<<(QDataStream& s,
                                 const JamendoDynamicPlaylist& p);
  friend QDataStream& operator>>(QDataStream& s, JamendoDynamicPlaylist& p);

 public:
  // These values are persisted - only add to the end
  enum OrderBy {
    OrderBy_Rating = 0,
    OrderBy_RatingWeek = 1,
    OrderBy_RatingMonth = 2,
    OrderBy_Listened = 3,
  };

  // These values are persisted - only add to the end
  enum OrderDirection {
    Order_Ascending = 0,
    Order_Descending = 1,
  };

  JamendoDynamicPlaylist();
  JamendoDynamicPlaylist(const QString& name, OrderBy order_by);

  QString type() const { return "Jamendo"; }

  void Load(const QByteArray& data);
  void Load(OrderBy order_by,
            OrderDirection order_direction = Order_Descending);
  QByteArray Save() const;

  PlaylistItemList Generate();

  bool is_dynamic() const { return true; }
  PlaylistItemList GenerateMore(int count);

 private:
  void Fetch();
  static QString OrderSpec(OrderBy by, OrderDirection dir);

 private:
  OrderBy order_by_;
  OrderDirection order_direction_;

  int current_page_;
  PlaylistItemList current_items_;
  int current_index_;

  static const int kPageSize = 100;
  static const int kApiRetryLimit = 5;
  static const char* kUrl;
};

QDataStream& operator<<(QDataStream& s, const JamendoDynamicPlaylist& p);
QDataStream& operator>>(QDataStream& s, JamendoDynamicPlaylist& p);

#endif  // INTERNET_JAMENDO_JAMENDODYNAMICPLAYLIST_H_
