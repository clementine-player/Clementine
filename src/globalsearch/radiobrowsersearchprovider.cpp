/* This file is part of Clementine.
   Copyright 2021, Fabio Bas <ctrlaltca@gmail.com>

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

#include "radiobrowsersearchprovider.h"

#include "ui/iconloader.h"

namespace {
const int kSearchStationLimit = 10;
}  // namespace

RadioBrowserSearchProvider::RadioBrowserSearchProvider(
    Application* app, RadioBrowserService* service, QObject* parent)
    : SearchProvider(app, parent), service_(service) {
  Init(RadioBrowserService::kServiceName, "radiobrowser",
       IconLoader::Load("radiobrowser", IconLoader::Provider),
       WantsDelayedQueries);
  connect(service_,
          SIGNAL(SearchFinished(int, RadioBrowserService::StreamList)),
          SLOT(SearchFinishedSlot(int, RadioBrowserService::StreamList)));
}

void RadioBrowserSearchProvider::SearchAsync(int id, const QString& query) {
  PendingState state;
  state.orig_id_ = id;
  state.tokens_ = TokenizeQuery(query);

  const QString query_string = state.tokens_.join(" ");
  service_->Search(id, query_string, kSearchStationLimit);
}

void RadioBrowserSearchProvider::SearchFinishedSlot(
    int search_id, RadioBrowserService::StreamList streams) {
  ResultList ret;

  for (auto stream : streams) {
    Result result(this);
    result.group_automatically_ = false;
    result.metadata_ = stream.ToSong(QString());
    ret << result;
  }

  emit ResultsAvailable(search_id, ret);
  emit SearchFinished(search_id);
}
/*
void RadioBrowserSearchProvider::ShowConfig() {
  if (service_) {
    return service_->ShowConfig();
  }
}
*/