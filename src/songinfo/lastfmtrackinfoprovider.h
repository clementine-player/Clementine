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

#ifndef LASTFMTRACKINFOPROVIDER_H
#define LASTFMTRACKINFOPROVIDER_H

#include "songinfoprovider.h"

#ifdef HAVE_LIBSCROBBLER
#include <scrobbler/XmlQuery.h>
namespace lastfm {
using scrobbler::XmlQuery;
}
#else
namespace lastfm {
class XmlQuery;
}
#endif

class QNetworkReply;

class LastfmTrackInfoProvider : public SongInfoProvider {
  Q_OBJECT

 public:
  void FetchInfo(int id, const Song& metadata);

 private slots:
  void RequestFinished();

 private:
  void GetPlayCounts(int id, const lastfm::XmlQuery& q);
  void GetWiki(int id, const lastfm::XmlQuery& q);
  void GetTags(int id, const lastfm::XmlQuery& q);

 private:
  QMap<QNetworkReply*, int> requests_;
};

#endif  // LASTFMTRACKINFOPROVIDER_H
