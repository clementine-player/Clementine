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

#ifndef SPOTIFYSEARCHPLAYLISTTYPE_H
#define SPOTIFYSEARCHPLAYLISTTYPE_H

#include "playlist/specialplaylisttype.h"

class SpotifyService;

class SpotifySearchPlaylistType : public SpecialPlaylistType {
public:
  SpotifySearchPlaylistType(SpotifyService* service);

  static const char* kName;
  virtual QString name() const { return kName; }

  virtual QIcon icon(Playlist* playlist) const;
  virtual QString search_hint_text(Playlist* playlist) const;
  virtual QString empty_playlist_text(Playlist* playlist) const;
  virtual QString playlist_view_css(Playlist* playlist) const;

  virtual bool has_special_search_behaviour(Playlist* playlist) const;
  virtual void Search(const QString& text, Playlist* playlist);
  virtual void DidYouMeanClicked(const QString& text, Playlist* playlist);

private:
  SpotifyService* service_;
};

#endif // SPOTIFYSEARCHPLAYLISTTYPE_H
