#ifndef JAMENDODYNAMICPLAYLIST_H
#define JAMENDODYNAMICPLAYLIST_H

#include "smartplaylists/generator.h"

class JamendoDynamicPlaylist : public smart_playlists::Generator {
  Q_OBJECT
  friend QDataStream& operator <<(QDataStream& s, const JamendoDynamicPlaylist& p);
  friend QDataStream& operator >>(QDataStream& s, JamendoDynamicPlaylist& p);

public:
  JamendoDynamicPlaylist();

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

  QString type() const { return "Jamendo"; }

  void Load(const QByteArray& data);
  void Load(OrderBy order_by, OrderDirection order_direction = Order_Descending);
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
  static const char* kUrl;
};

QDataStream& operator <<(QDataStream& s, const JamendoDynamicPlaylist& p);
QDataStream& operator >>(QDataStream& s, JamendoDynamicPlaylist& p);

#endif
