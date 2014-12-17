/* This file is part of Clementine.
   Copyright 2011, Arnaud Bienner <arnaud.bienner@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>
   Copyright 2014, John Maguire <john.maguire@gmail.com>

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

#ifndef INTERNET_GROOVESHARKRADIO_H_
#define INTERNET_GROOVESHARKRADIO_H_

#include "smartplaylists/generator.h"

class GroovesharkService;

class GroovesharkRadio : public smart_playlists::Generator {
 public:
  // Start Grooveshark radio for a particular type of music
  GroovesharkRadio(GroovesharkService* service, int tag_id);
  // Start Grooveshark radio based on last artists and songs you listen to
  explicit GroovesharkRadio(GroovesharkService* service);

  QString type() const { return "Grooveshark"; }
  void Load(const QByteArray& data);
  QByteArray Save() const;
  PlaylistItemList Generate();
  PlaylistItemList GenerateMore(int count) { return Generate(); }
  bool is_dynamic() const { return true; }

 private:
  GroovesharkService* service_;
  int tag_id_;
  // Boolean to specify if we should use tag. If not, we will used autoplay
  // without tag
  bool use_tag_;
  // For Generate: indicates if it's the first time we generate songs
  bool first_time_;
  QVariantMap autoplay_state_;
};

#endif  // INTERNET_GROOVESHARKRADIO_H_
