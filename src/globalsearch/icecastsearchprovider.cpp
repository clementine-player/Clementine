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

#include "icecastsearchprovider.h"
#include "internet/icecastbackend.h"

IcecastSearchProvider::IcecastSearchProvider(IcecastBackend* backend, QObject* parent)
  : BlockingSearchProvider(parent),
    backend_(backend)
{
  Init("Icecast", "icecast", QIcon(":last.fm/icon_radio.png"));
}

SearchProvider::ResultList IcecastSearchProvider::Search(int id, const QString& query) {
  IcecastBackend::StationList stations = backend_->GetStations(query);
  ResultList ret;

  const QStringList tokens = TokenizeQuery(query);

  foreach (const IcecastBackend::Station& station, stations) {
    if (ret.count() > 3)
      break;

    Result result(this);
    result.type_ = globalsearch::Type_Stream;
    result.metadata_ = station.ToSong();
    result.match_quality_ = MatchQuality(tokens, station.name);
    ret << result;
  }

  return ret;
}
