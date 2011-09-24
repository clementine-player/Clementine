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

#include "lastfmsearchprovider.h"
#include "core/logging.h"
#include "internet/lastfmservice.h"
#include "playlist/songmimedata.h"

const int LastFMSearchProvider::kResultLimit = 6;


LastFMSearchProvider::LastFMSearchProvider(LastFMService* service, QObject* parent)
  : BlockingSearchProvider(parent),
    service_(service) {
  Init("Last.fm", "lastfm", QIcon(":last.fm/as.png"), false, true);
  icon_ = ScaleAndPad(QImage(":last.fm/as.png"));

  safe_words_ << "lastfm" << "last.fm";

  connect(service, SIGNAL(SavedItemsChanged()), SLOT(RecreateItems()));
  RecreateItems();
}

SearchProvider::ResultList LastFMSearchProvider::Search(int id, const QString& query) {
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

      int index = item.metadata_.title().indexOf(token, 0, Qt::CaseInsensitive);
      if (index == -1) {
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
      result.match_quality_ = MatchQuality(tokens, result.metadata_.title());
    }
    if (result.match_quality_ != Result::Quality_None) {
      result.metadata_ = item.metadata_;
      ret << result;
    }

    if (ret.count() >= kResultLimit)
      break;
  }

  return ret;
}

void LastFMSearchProvider::LoadArtAsync(int id, const Result& result) {
  // TODO: Maybe we should try to get user pictures for friends?

  emit ArtLoaded(id, icon_);
}

void LastFMSearchProvider::LoadTracksAsync(int id, const Result& result) {
  Song metadata = result.metadata_;
  metadata.set_filetype(Song::Type_Stream);

  SongMimeData* mime_data = new SongMimeData;
  mime_data->songs = SongList() << metadata;

  emit TracksLoaded(id, mime_data);
}

void LastFMSearchProvider::RecreateItems() {
  QList<Item> items;
  Item item;

  item.keyword_ = "recommended";
  item.metadata_.set_title(tr("My Last.fm Recommended Radio"));
  item.metadata_.set_url(QUrl("lastfm://user/USERNAME/recommended"));
  items << item;

  item.keyword_ = "radio";
  item.metadata_.set_title(tr("My Last.fm Library"));
  item.metadata_.set_url(QUrl("lastfm://user/USERNAME/library"));
  items << item;

  item.keyword_ = "mix";
  item.metadata_.set_title(tr("My Last.fm Mix Radio"));
  item.metadata_.set_url(QUrl("lastfm://user/USERNAME/mix"));
  items << item;

  item.keyword_ = "neighborhood";
  item.metadata_.set_title(tr("My Last.fm Neighborhood"));
  item.metadata_.set_url(QUrl("lastfm://user/USERNAME/neighbours"));
  items << item;

  const QStringList artists = service_->SavedArtistRadioNames();
  const QStringList tags = service_->SavedTagRadioNames();
  const QStringList friends = service_->FriendNames();

  foreach (const QString& name, artists) {
    item.keyword_ = name;
    item.metadata_.set_title(tr(LastFMService::kTitleArtist).arg(name));
    item.metadata_.set_url(QUrl(QString(LastFMService::kUrlArtist).arg(name)));
    items << item;
  }

  foreach (const QString& name, tags) {
    item.keyword_ = name;
    item.metadata_.set_title(tr(LastFMService::kTitleTag).arg(name));
    item.metadata_.set_url(QUrl(QString(LastFMService::kUrlTag).arg(name)));
    items << item;
  }

  foreach (const QString& name, friends) {
    item.keyword_ = name;

    item.metadata_.set_title(tr("Last.fm Radio Station - %1").arg(name));
    item.metadata_.set_url(QUrl("lastfm://user/" + name + "/library"));
    items << item;

    item.metadata_.set_title(tr("Last.fm Mix Radio - %1").arg(name));
    item.metadata_.set_url(QUrl("lastfm://user/" + name + "/mix"));
    items << item;

    item.metadata_.set_title(tr("Last.fm Neighbor Radio - %1").arg(name));
    item.metadata_.set_url(QUrl("lastfm://user/" + name + "/neighbours"));
    items << item;
  }

  QMutexLocker l(&items_mutex_);
  items_ = items;
}
