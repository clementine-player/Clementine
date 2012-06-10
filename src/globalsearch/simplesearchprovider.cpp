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

#include "simplesearchprovider.h"
#include "core/logging.h"
#include "playlist/songmimedata.h"


const int SimpleSearchProvider::kDefaultResultLimit = 6;

SimpleSearchProvider::Item::Item(const QString& title, const QUrl& url, const QString& keyword)
  : keyword_(keyword)
{
  metadata_.set_title(title);
  metadata_.set_url(url);
}

SimpleSearchProvider::Item::Item(const Song& song, const QString& keyword)
  : keyword_(keyword),
    metadata_(song)
{
}


SimpleSearchProvider::SimpleSearchProvider(Application* app, QObject* parent)
  : BlockingSearchProvider(app, parent),
    result_limit_(kDefaultResultLimit),
    items_dirty_(true),
    has_searched_before_(false)
{
}

void SimpleSearchProvider::MaybeRecreateItems() {
  if (has_searched_before_) {
    RecreateItems();
  } else {
    items_dirty_ = true;
  }
}

SearchProvider::ResultList SimpleSearchProvider::Search(int id, const QString& query) {
  Q_UNUSED(id)

  if (items_dirty_) {
    RecreateItems();
    items_dirty_ = false;
  }

  has_searched_before_ = true;

  ResultList ret;
  const QStringList tokens = TokenizeQuery(query);

  QMutexLocker l(&items_mutex_);
  foreach (const Item& item, items_) {
    bool matched = true;
    foreach (const QString& token, tokens) {
      if (!item.keyword_.contains(token, Qt::CaseInsensitive) &&
          !item.metadata_.title().contains(token, Qt::CaseInsensitive) &&
          !safe_words_.contains(token, Qt::CaseInsensitive)) {
        matched = false;
        break;
      }
    }

    if (matched) {
      Result result(this);
      result.group_automatically_ = false;
      result.metadata_ = item.metadata_;
      ret << result;
    }

    if (ret.count() >= result_limit_)
      break;
  }

  return ret;
}

void SimpleSearchProvider::LoadTracksAsync(int id, const Result& result) {
  Song metadata = result.metadata_;
  metadata.set_filetype(Song::Type_Stream);

  SongMimeData* mime_data = new SongMimeData;
  mime_data->songs = SongList() << metadata;

  emit TracksLoaded(id, mime_data);
}

void SimpleSearchProvider::SetItems(const ItemList& items) {
  QMutexLocker l(&items_mutex_);
  items_ = items;
}

QString SimpleSearchProvider::GetSuggestion() {
  QMutexLocker l(&items_mutex_);

  if (items_.isEmpty())
    return QString();

  for (int attempt=0 ; attempt<10 ; ++attempt) {
    const Item& item = items_[qrand() % items_.count()];
    if (!item.keyword_.isEmpty())
      return item.keyword_;
    if (!item.metadata_.title().isEmpty())
      return item.metadata_.title();
  }

  return QString();
}
