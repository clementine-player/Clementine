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

#include "internet/icecast/icecastbackend.h"
#include "ui/iconloader.h"

IcecastSearchProvider::IcecastSearchProvider(
    std::shared_ptr<IcecastBackend> backend, Application* app, QObject* parent)
    : BlockingSearchProvider(app, parent), backend_(backend) {
  Init("Icecast", "icecast", IconLoader::Load("icon_radio", IconLoader::Lastfm),
       DisabledByDefault);
}

SearchProvider::ResultList IcecastSearchProvider::Search(int id,
                                                         const QString& query) {
  IcecastBackend::StationList stations = backend_->GetStations(query);
  ResultList ret;

  for (const IcecastBackend::Station& station : stations) {
    if (ret.count() > 3) break;

    Result result(this);
    result.group_automatically_ = false;
    result.metadata_ = station.ToSong();
    ret << result;
  }

  return ret;
}
