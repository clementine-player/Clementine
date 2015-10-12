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

#include "urlsearchprovider.h"
#include "core/mimedata.h"
#include "ui/iconloader.h"

#include <QIcon>
#include <QUrl>

const char* UrlSearchProvider::kUrlRegex = "^[a-zA-Z][a-zA-Z0-9+-.]*://";

UrlSearchProvider::UrlSearchProvider(Application* app, QObject* parent)
    : SearchProvider(app, parent), url_regex_(kUrlRegex) {
  QIcon icon = IconLoader::Load("applications-internet", IconLoader::Base);
  image_ = ScaleAndPad(icon.pixmap(kArtHeight, kArtHeight).toImage());

  Init("URL", "url", icon, MimeDataContainsUrlsOnly);
}

void UrlSearchProvider::SearchAsync(int id, const QString& query) {
  Result result(this);
  result.group_automatically_ = false;
  result.metadata_.set_url(QUrl::fromUserInput(query));
  result.metadata_.set_title(result.metadata_.url().toString());
  result.metadata_.set_filetype(Song::Type_Stream);

  emit ResultsAvailable(id, ResultList() << result);
}

void UrlSearchProvider::LoadArtAsync(int id, const Result&) {
  emit ArtLoaded(id, image_);
}

bool UrlSearchProvider::LooksLikeUrl(const QString& query) const {
  return url_regex_.indexIn(query) == 0;
}
