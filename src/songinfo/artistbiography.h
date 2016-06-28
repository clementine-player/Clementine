/* This file is part of Clementine.
   Copyright 2016, John Maguire <john.maguire@gmail.com>

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

#ifndef ARTISTBIOGRAPHY_H
#define ARTISTBIOGRAPHY_H

#include <memory>

#include "songinfoprovider.h"

class NetworkAccessManager;

class ArtistBiography : public SongInfoProvider {
  Q_OBJECT

 public:
  ArtistBiography();
  ~ArtistBiography();

  void FetchInfo(int id, const Song& metadata) override;

 private:
  void FetchWikipediaImages(int id, const QString& title);

  std::unique_ptr<NetworkAccessManager> network_;
};

#endif  // ARTISTBIOGRAPHY_H
