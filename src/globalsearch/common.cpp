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

#include "common.h"

#ifdef HAVE_DBUS

QDBusArgument& operator <<(QDBusArgument& arg, const GlobalSearchServiceResult& result) {
  arg.beginStructure();
  arg << result.result_id_
      << result.art_on_the_way_
      << result.provider_name_
      << result.type_
      << result.match_quality_
      << result.album_size_
      << result.title_
      << result.artist_
      << result.album_
      << result.album_artist_
      << result.is_compilation_
      << result.track_;
  arg.endStructure();

  return arg;
}

const QDBusArgument& operator >>(const QDBusArgument& arg, GlobalSearchServiceResult& result) {
  int type;
  int match_quality;

  arg.beginStructure();
  arg >> result.result_id_
      >> result.art_on_the_way_
      >> result.provider_name_
      >> type
      >> match_quality
      >> result.album_size_
      >> result.title_
      >> result.artist_
      >> result.album_
      >> result.album_artist_
      >> result.is_compilation_
      >> result.track_;
  arg.endStructure();

  result.type_ = static_cast<globalsearch::Type>(type);
  result.match_quality_ = static_cast<globalsearch::MatchQuality>(match_quality);

  return arg;
}

#endif // HAVE_DBUS
