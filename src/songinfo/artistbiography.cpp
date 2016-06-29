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
#include "core/latch.h"
#include "core/logging.h"
#include "core/network.h"
#include "songinfo/songinfotextview.h"

namespace {
const char* kArtistBioUrl = "https://data.clementine-player.org/fetchbio";
const char* kWikipediaImageListUrl =
    "https://%1.wikipedia.org/w/"
    "api.php?action=query&prop=images&format=json&imlimit=25";
const char* kWikipediaImageInfoUrl =
    "https://%1.wikipedia.org/w/"
    "api.php?action=query&prop=imageinfo&iiprop=url|size&format=json";
const int kMinimumImageSize = 400;

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

  qLog(Debug) << "Biography url: " << url;

  QNetworkRequest request(url);
  QNetworkReply* reply = network_->get(request);

  NewClosure(reply, SIGNAL(finished()), [this, reply, id]() {
    reply->deleteLater();

    QJson::Parser parser;
    QVariantMap response = parser.parse(reply).toMap();

    QString body = response["articleBody"].toString();
    QString url = response["url"].toString();

    if (!body.isEmpty()) {
      CollapsibleInfoPane::Data data;
      data.id_ = url;
      data.title_ = tr("Biography");
      data.type_ = CollapsibleInfoPane::Data::Type_Biography;

      QString text;
      text += "<p><a href=\"" + url + "\">" + tr("Open in your browser") +
              "</a></p>";

      text += body;
      SongInfoTextView* editor = new SongInfoTextView;
      editor->SetHtml(text);
      data.contents_ = editor;
      emit InfoReady(id, data);
    }

    if (url.contains("wikipedia.org")) {
      FetchWikipediaImages(id, url);
    } else {
      emit Finished(id);
    }
  });
}

namespace {

QStringList ExtractImageTitles(const QVariantMap& json) {
  QStringList ret;
  for (auto it = json.constBegin(); it != json.constEnd(); ++it) {
    if (it.value().type() == QVariant::Map) {
      ret.append(ExtractImageTitles(it.value().toMap()));
    } else if (it.key() == "images" && it.value().type() == QVariant::List) {
      QVariantList images = it.value().toList();
      for (QVariant i : images) {
        QVariantMap image = i.toMap();
        QString image_title = image["title"].toString();
        if (!image_title.isEmpty() &&
            (
                // SVGs tend to be irrelevant icons.
                image_title.endsWith(".jpg", Qt::CaseInsensitive) ||
                image_title.endsWith(".png", Qt::CaseInsensitive))) {
          ret.append(image_title);
        }
      }
    }
  }
  return ret;
}

QUrl ExtractImageUrl(const QVariantMap& json) {
  for (auto it = json.constBegin(); it != json.constEnd(); ++it) {
    if (it.value().type() == QVariant::Map) {
      QUrl r = ExtractImageUrl(it.value().toMap());
      if (!r.isEmpty()) {
        return r;
      }
    } else if (it.key() == "imageinfo") {
      QVariantList imageinfos = it.value().toList();
      QVariantMap imageinfo = imageinfos.first().toMap();
      int width = imageinfo["width"].toInt();
      int height = imageinfo["height"].toInt();
      if (width < kMinimumImageSize || height < kMinimumImageSize) {
        return QUrl();
      }
      return QUrl::fromEncoded(imageinfo["url"].toByteArray());
    }
  }
  return QUrl();
}

}  // namespace

void ArtistBiography::FetchWikipediaImages(int id,
                                           const QString& wikipedia_url) {
  qLog(Debug) << wikipedia_url;
  QRegExp regex("([a-z]+)\\.wikipedia\\.org/wiki/(.*)");
  if (regex.indexIn(wikipedia_url) == -1) {
    emit Finished(id);
    return;
  }
  QString wiki_title = QUrl::fromPercentEncoding(regex.cap(2).toUtf8());
  QString language = regex.cap(1);
  QUrl url(QString(kWikipediaImageListUrl).arg(language));
  url.addQueryItem("titles", wiki_title);

  qLog(Debug) << "Wikipedia images:" << url;

  QNetworkRequest request(url);
  QNetworkReply* reply = network_->get(request);
  NewClosure(reply, SIGNAL(finished()), [this, id, reply, language]() {
    reply->deleteLater();

    QJson::Parser parser;
    QVariantMap response = parser.parse(reply).toMap();

    QStringList image_titles = ExtractImageTitles(response);

    CountdownLatch* latch = new CountdownLatch;
    NewClosure(latch, SIGNAL(Done()), [this, latch, id]() {
      latch->deleteLater();
      emit Finished(id);
    });

    for (const QString& image_title : image_titles) {
      latch->Wait();
      QUrl url(QString(kWikipediaImageInfoUrl).arg(language));
      url.addQueryItem("titles", image_title);
      qLog(Debug) << "Image info:" << url;

      QNetworkRequest request(url);
      QNetworkReply* reply = network_->get(request);
      NewClosure(reply, SIGNAL(finished()), [this, id, reply, latch]() {
        reply->deleteLater();
        QJson::Parser parser;
        QVariantMap json = parser.parse(reply).toMap();
        QUrl url = ExtractImageUrl(json);
        qLog(Debug) << "Found wikipedia image url:" << url;
        if (!url.isEmpty()) {
          emit ImageReady(id, url);
        }
        latch->CountDown();
      });
    }
  });
}
