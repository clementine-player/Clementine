/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>

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

#ifndef TWITTERARTISTINFO_H
#define TWITTERARTISTINFO_H

#include "songinfoprovider.h"

#include "core/network.h"

class TwitterArtistInfo : public SongInfoProvider {
  Q_OBJECT
 public:
  TwitterArtistInfo();

  void FetchInfo(int id, const Song& metadata);

 private slots:
  void ArtistSearchFinished(QNetworkReply* reply, int id);
  void UserTimelineRequestFinished(
      QNetworkReply* reply, const QString& twitter_id, int id);

 private:
  void FetchUserTimeline(const QString& twitter_id, int id);

  NetworkAccessManager network_;

  static const char* kTwitterBucket;
  static const char* kTwitterTimelineUrl;
};

#endif
