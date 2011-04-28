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

#ifndef SPECIALPLAYLISTTYPE_H
#define SPECIALPLAYLISTTYPE_H

#include <QIcon>
#include <QString>

class Playlist;


class SpecialPlaylistType {
public:
  virtual ~SpecialPlaylistType() {}

  virtual QString name() const = 0;

  virtual QIcon icon(Playlist* playlist) const;
  virtual QString search_hint_text(Playlist* playlist) const;
  virtual QString empty_playlist_text(Playlist* playlist) const;
  virtual QString playlist_view_css(Playlist* playlist) const;

  virtual bool has_special_search_behaviour(Playlist* playlist) const;
  virtual void Search(const QString& text, Playlist* playlist);
};


class DefaultPlaylistType : public SpecialPlaylistType {
public:
  virtual QString name() const { return QString(); }
};

#endif // SPECIALPLAYLISTTYPE_H
