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


SimpleSearchProvider::SimpleSearchProvider(QObject* parent)
  : BlockingSearchProvider(parent),
    result_limit_(kDefaultResultLimit)
{
}

SearchProvider::ResultList SimpleSearchProvider::Search(int id, const QString& query) {
  Q_UNUSED(id)

  ResultList ret;
  const QStringList tokens = TokenizeQuery(query);

  QMutexLocker l(&items_mutex_);
  foreach (const Item& item, items_) {
    Result result(this);
    result.type_ = Result::Type_Stream;
    result.match_quality_ = Result::Quality_None;

    foreach (const QString& token, tokens) {
      if (item.keyword_.startsWith(token, Qt::CaseInsensitive)) {
        result.match_quality_ = Result::Quality_AtStart;
        continue;
      }

      if (!item.metadata_.title().contains(token, Qt::CaseInsensitive)) {
        bool matched_safe_word = false;
        foreach (const QString& safe_word, safe_words_) {
          if (safe_word.startsWith(token, Qt::CaseInsensitive)) {
            matched_safe_word = true;
            break;
          }
        }

        if (matched_safe_word)
          continue;
        result.match_quality_ = Result::Quality_None;
        break;
      }

      result.match_quality_ = qMin(result.match_quality_, Result::Quality_Middle);
    }

    if (result.match_quality_ == Result::Quality_Middle) {
      result.match_quality_ = MatchQuality(tokens, item.metadata_.title());
    }
    if (result.match_quality_ != Result::Quality_None) {
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
