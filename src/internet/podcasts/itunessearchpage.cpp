/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>
   Copyright 2012, 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>

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

#include "itunessearchpage.h"

#include <qjson/parser.h>
#include <QMessageBox>
#include <QNetworkReply>

#include "core/closure.h"
#include "core/network.h"
#include "podcast.h"
#include "podcastdiscoverymodel.h"
#include "ui_itunessearchpage.h"
#include "ui/iconloader.h"

const char* ITunesSearchPage::kUrlBase =
    "http://ax.phobos.apple.com.edgesuite.net/WebObjects/MZStoreServices.woa/"
    "wa/wsSearch?country=US&media=podcast";

ITunesSearchPage::ITunesSearchPage(Application* app, QWidget* parent)
    : AddPodcastPage(app, parent),
      ui_(new Ui_ITunesSearchPage),
      network_(new NetworkAccessManager(this)) {
  ui_->setupUi(this);
  connect(ui_->search, SIGNAL(clicked()), SLOT(SearchClicked()));
  setWindowIcon(IconLoader::Load("itunes", IconLoader::Provider));
}

ITunesSearchPage::~ITunesSearchPage() { delete ui_; }

void ITunesSearchPage::SearchClicked() {
  emit Busy(true);

  QUrl url(QUrl::fromEncoded(kUrlBase));
  url.addQueryItem("term", ui_->query->text());

  QNetworkReply* reply = network_->get(QNetworkRequest(url));
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(SearchFinished(QNetworkReply*)), reply);
}

void ITunesSearchPage::SearchFinished(QNetworkReply* reply) {
  reply->deleteLater();
  emit Busy(false);

  model()->clear();

  // Was there a network error?
  if (reply->error() != QNetworkReply::NoError) {
    QMessageBox::warning(this, tr("Failed to fetch podcasts"),
                         reply->errorString());
    return;
  }

  QJson::Parser parser;
  QVariant data = parser.parse(reply);

  // Was it valid JSON?
  if (data.isNull()) {
    QMessageBox::warning(
        this, tr("Failed to fetch podcasts"),
        tr("There was a problem parsing the response from the iTunes Store"));
    return;
  }

  // Was there an error message in the JSON?
  if (data.toMap().contains("errorMessage")) {
    QMessageBox::warning(this, tr("Failed to fetch podcasts"),
                         data.toMap()["errorMessage"].toString());
    return;
  }

  for (const QVariant& result_variant : data.toMap()["results"].toList()) {
    QVariantMap result(result_variant.toMap());
    if (result["kind"].toString() != "podcast") {
      continue;
    }

    Podcast podcast;
    podcast.set_author(result["artistName"].toString());
    podcast.set_title(result["trackName"].toString());
    podcast.set_url(result["feedUrl"].toUrl());
    podcast.set_link(result["trackViewUrl"].toUrl());
    podcast.set_image_url_small(QUrl(result["artworkUrl30"].toString()));
    podcast.set_image_url_large(QUrl(result["artworkUrl100"].toString()));

    model()->appendRow(model()->CreatePodcastItem(podcast));
  }
}

void ITunesSearchPage::Show() { ui_->query->setFocus(); }
