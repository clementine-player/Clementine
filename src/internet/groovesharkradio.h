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

#ifndef GROOVESHARKDYNAMICPLAYLIST_H
#define GROOVESHARKDYNAMICPLAYLIST_H

#include "smartplaylists/generator.h"

class GroovesharkService;

class GroovesharkRadio : public smart_playlists::Generator {

public:
  GroovesharkRadio(GroovesharkService* service, int tag_id);

  QString type() const { return "Grooveshark"; }
  void Load(const QByteArray& data);
  QByteArray Save() const;
  PlaylistItemList Generate();
  PlaylistItemList GenerateMore(int count) { return Generate(); }
  bool is_dynamic() const { return true; }

private:
  GroovesharkService* service_;
  int tag_id_;
  // For Generate: indicates if it's the first time we generate songs
  bool first_time_;
  QVariantMap autoplay_state_;
};
 
#endif // GROOVESHARKDYNAMICPLAYLIST_H
