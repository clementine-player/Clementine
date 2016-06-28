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

#include "artistbiography.h"

#include <QLocale>

#include <qjson/parser.h>

#include "core/closure.h"
#include "core/network.h"
#include "songinfo/songinfotextview.h"

namespace {
const char* kArtistBioUrl = "https://data.clementine-player.org/fetchbio";

QString GetLocale() {
  QLocale locale;
  return locale.name().split('_')[0];
}

}  // namespace

ArtistBiography::ArtistBiography() : network_(new NetworkAccessManager) {}

ArtistBiography::~ArtistBiography() {}

void ArtistBiography::FetchInfo(int id, const Song& metadata) {
  if (metadata.artist().isEmpty()) {
    emit Finished(id);
    return;
  }

  QUrl url(kArtistBioUrl);
  url.addQueryItem("artist", metadata.artist());
  url.addQueryItem("lang", GetLocale());

  QNetworkRequest request(url);
  QNetworkReply* reply = network_->get(request);

  NewClosure(reply, SIGNAL(finished()), [this, reply, id]() {
    reply->deleteLater();

    QJson::Parser parser;
    QVariantMap response = parser.parse(reply).toMap();

    QString body = response["articleBody"].toString();
    QString url = response["url"].toString();

    CollapsibleInfoPane::Data data;
    data.id_ = url;
    data.title_ = tr("Biography");
    data.type_ = CollapsibleInfoPane::Data::Type_Biography;

    QString text;
    text +=
        "<p><a href=\"" + url + "\">" + tr("Open in your browser") + "</a></p>";

    text += body;
    SongInfoTextView* editor = new SongInfoTextView;
    editor->SetHtml(text);
    data.contents_ = editor;
    emit InfoReady(id, data);
    emit Finished(id);
  });
}
