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

LastFMSearchProvider::LastFMSearchProvider(LastFMService* service,
                                           Application* app, QObject* parent)
    : SimpleSearchProvider(app, parent), service_(service) {
  Init("Last.fm", "lastfm", QIcon(":last.fm/as.png"),
       CanShowConfig | CanGiveSuggestions);
  icon_ = ScaleAndPad(QImage(":last.fm/as.png"));

  set_safe_words(QStringList() << "lastfm"
                               << "last.fm");
  set_max_suggestion_count(3);

  connect(service, SIGNAL(SavedItemsChanged()), SLOT(MaybeRecreateItems()));

  // Load the friends list on startup only if it doesn't involve going to update
  // info from the server.
  if (!service_->IsFriendsListStale()) RecreateItems();
}

void LastFMSearchProvider::LoadArtAsync(int id, const Result& result) {
  // TODO: Maybe we should try to get user pictures for friends?

  emit ArtLoaded(id, icon_);
}

void LastFMSearchProvider::RecreateItems() {
  QList<Item> items;

  items << Item(tr("My Last.fm Recommended Radio"),
                QUrl("lastfm://user/USERNAME/recommended"), "recommended");
  items << Item(tr("My Last.fm Library"),
                QUrl("lastfm://user/USERNAME/library"), "radio");
  items << Item(tr("My Last.fm Mix Radio"), QUrl("lastfm://user/USERNAME/mix"),
                "mix");
  items << Item(tr("My Last.fm Neighborhood"),
                QUrl("lastfm://user/USERNAME/neighbours"), "neighborhood");

  const QStringList artists = service_->SavedArtistRadioNames();
  const QStringList tags = service_->SavedTagRadioNames();
  const QStringList friends = service_->FriendNames();

  foreach(const QString & name, artists) {
    items << Item(tr(LastFMService::kTitleArtist).arg(name),
                  QUrl(QString(LastFMService::kUrlArtist).arg(name)), name);
  }

  foreach(const QString & name, tags) {
    items << Item(tr(LastFMService::kTitleTag).arg(name),
                  QUrl(QString(LastFMService::kUrlTag).arg(name)), name);
  }

  foreach(const QString & name, friends) {
    items << Item(tr("Last.fm Radio Station - %1").arg(name),
                  QUrl("lastfm://user/" + name + "/library"), name);
    items << Item(tr("Last.fm Mix Radio - %1").arg(name),
                  QUrl("lastfm://user/" + name + "/mix"), name);
    items << Item(tr("Last.fm Neighbor Radio - %1").arg(name),
                  QUrl("lastfm://user/" + name + "/neighbours"), name);
  }

  SetItems(items);
}

bool LastFMSearchProvider::IsLoggedIn() { return service_->IsAuthenticated(); }

void LastFMSearchProvider::ShowConfig() { service_->ShowConfig(); }
