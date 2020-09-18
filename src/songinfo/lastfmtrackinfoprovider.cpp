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

#include "lastfmtrackinfoprovider.h"

#include "internet/lastfm/lastfmcompat.h"
#include "songinfotextview.h"
#include "songplaystats.h"
#include "tagwidget.h"
#include "ui/iconloader.h"

void LastfmTrackInfoProvider::FetchInfo(int id, const Song& metadata) {
  QMap<QString, QString> params;
  params["method"] = "track.getInfo";
  params["track"] = metadata.title();
  params["artist"] = metadata.artist();

  if (!lastfm::ws::Username.isEmpty())
    params["username"] = lastfm::ws::Username;

  QNetworkReply* reply = lastfm::ws::get(params);
  connect(reply, SIGNAL(finished()), SLOT(RequestFinished()));
  requests_[reply] = id;
}

void LastfmTrackInfoProvider::RequestFinished() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (!reply || !requests_.contains(reply)) return;

  const int id = requests_.take(reply);

  if (reply->error() != QNetworkReply::NoError) {
    emit Finished(id);
    return;
  }

  lastfm::XmlQuery query(lastfm::compat::EmptyXmlQuery());
  if (lastfm::compat::ParseQuery(reply->readAll(), &query)) {
    GetPlayCounts(id, query);
    GetWiki(id, query);
    GetTags(id, query);
  }

  emit Finished(id);
}

void LastfmTrackInfoProvider::GetPlayCounts(int id, const lastfm::XmlQuery& q) {
  // Parse the response
  const int listeners = q["track"]["listeners"].text().toInt();
  const int playcount = q["track"]["playcount"].text().toInt();
  int myplaycount = -1;
  bool love = false;

  if (!q["track"].children("userplaycount").isEmpty()) {
    myplaycount = q["track"]["userplaycount"].text().toInt();
    love = q["track"]["userloved"].text() == "1";
  }

  if (!listeners && !playcount && myplaycount == -1) return;  // No useful data

  CollapsibleInfoPane::Data data;
  data.id_ = "lastfm/playcounts";
  data.title_ = tr("Last.fm play counts");
  data.type_ = CollapsibleInfoPane::Data::Type_PlayCounts;
  data.icon_ = IconLoader::Load("as", IconLoader::Lastfm);

  SongPlayStats* widget = new SongPlayStats;
  data.contents_ = widget;

  if (myplaycount != -1) {
    if (love)
      widget->AddItem(IconLoader::Load("love", IconLoader::Lastfm),
                      tr("You love this track"));
    widget->AddItem(IconLoader::Load("icon_user", IconLoader::Lastfm),
                    tr("Your scrobbles: %1").arg(myplaycount));
  }

  if (playcount)
    widget->AddItem(IconLoader::Load("media-playback-start", IconLoader::Base),
                    tr("%L1 total plays").arg(playcount));
  if (listeners)
    widget->AddItem(IconLoader::Load("my_neighbours", IconLoader::Lastfm),
                    tr("%L1 other listeners").arg(listeners));

  emit InfoReady(id, data);
}

void LastfmTrackInfoProvider::GetWiki(int id, const lastfm::XmlQuery& q) {
  // Parse the response
  if (q["track"].children("wiki").isEmpty()) return;  // No wiki element

  const QString content = q["track"]["wiki"]["content"].text();

  if (content.isEmpty()) return;  // No useful data

  CollapsibleInfoPane::Data data;
  data.id_ = "lastfm/songwiki";
  data.title_ = tr("Last.fm wiki");
  data.type_ = CollapsibleInfoPane::Data::Type_Biography;
  data.icon_ = IconLoader::Load("as", IconLoader::Lastfm);

  SongInfoTextView* widget = new SongInfoTextView;
  data.contents_ = widget;

  widget->SetHtml(content);

  emit InfoReady(id, data);
}

void LastfmTrackInfoProvider::GetTags(int id, const lastfm::XmlQuery& q) {
  // Parse the response
  if (q["track"].children("toptags").isEmpty() ||
      q["track"]["toptags"].children("tag").isEmpty())
    return;  // No tag elements

  CollapsibleInfoPane::Data data;
  data.id_ = "lastfm/songtags";
  data.title_ = tr("Last.fm tags");
  data.type_ = CollapsibleInfoPane::Data::Type_Biography;
  data.icon_ = IconLoader::Load("icon_tag", IconLoader::Lastfm);

  TagWidget* widget = new TagWidget(TagWidget::Type_Tags);
  data.contents_ = widget;

  widget->SetIcon(data.icon_);

  for (const lastfm::XmlQuery& e : q["track"]["toptags"].children("tag")) {
    widget->AddTag(e["name"].text());
  }

  emit InfoReady(id, data);
}
