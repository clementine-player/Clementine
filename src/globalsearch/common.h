/* This file is part of Clementine.
   Copyright 2011, David Sansome <me@davidsansome.com>

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

#ifndef GLOBALSEARCHSERVICERESULT_H
#define GLOBALSEARCHSERVICERESULT_H

// This file contains definitions that are shared between Clementine and any
// external applications that need to do global searches over DBus.

#include "config.h"

#ifdef HAVE_DBUS
#  include <QDBusArgument>
#endif

#include <QIcon>

namespace globalsearch {

// The order of types here is the order they'll appear in the UI.
enum Type {
  Type_Track = 0,
  Type_Stream,
  Type_Album
};

enum MatchQuality {
  // A token in the search string matched at the beginning of the song
  // metadata.
  Quality_AtStart = 0,

  // A token matched somewhere else.
  Quality_Middle,

  Quality_None
};

} // namespace globalsearch


#ifdef HAVE_DBUS

struct GlobalSearchServiceResult {
  // When adding new fields to this struct remember to update the dbus signature
  // which is duplicated in the xml specification and in clementinerunner.cpp

  int result_id_;
  bool art_on_the_way_;

  QString provider_name_;
  globalsearch::Type type_;
  globalsearch::MatchQuality match_quality_;

  int album_size_;

  QString title_;
  QString artist_;
  QString album_;
  QString album_artist_;
  bool is_compilation_;
  int track_;

  // Not included in the dbus emission.
  QIcon image_;
  int provider_order_;
};
typedef QList<GlobalSearchServiceResult> GlobalSearchServiceResultList;

Q_DECLARE_METATYPE(GlobalSearchServiceResult)
Q_DECLARE_METATYPE(GlobalSearchServiceResultList)

QDBusArgument& operator <<(QDBusArgument& arg, const GlobalSearchServiceResult& result);
const QDBusArgument& operator >>(const QDBusArgument& arg, GlobalSearchServiceResult& result);

#endif // HAVE_DBUS


#endif // GLOBALSEARCHSERVICERESULT_H
