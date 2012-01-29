/* This file is part of Clementine.
   Copyright 2012, Martin Björklund <mbj4668@gmail.com>

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

#include "discogscoverprovider.h"
#include "core/closure.h"
#include "core/logging.h"
#include "core/network.h"
#include "core/utilities.h"

#include <QNetworkReply>
#include <qjson/parser.h>

const char* DiscogsCoverProvider::kSearchUrl = "http://api.discogs.com/database/search";

DiscogsCoverProvider::DiscogsCoverProvider(QObject* parent)
  : CoverProvider("Discogs", parent),
    network_(new NetworkAccessManager(this))
{
}

// Send the search request using Discog's HTTP-based API.
bool DiscogsCoverProvider::StartSearch(const QString& artist, const QString& album, int id) {
  typedef QPair<QString, QString> Arg;
  typedef QList<Arg> ArgList;

  typedef QPair<QByteArray, QByteArray> EncodedArg;
  typedef QList<EncodedArg> EncodedArgList;

  QUrl url(kSearchUrl);

  // For now, search only for the master release.  Most releases have a master, but not all...
  // In order to handle other releases we either could search for all
  // types and handle the master in SearchReply (then we'd have to handle paging), or search again
  // if no master exists.  In both cases, we'd have to remember some data on the heap in order to send
  // more requests.
  ArgList args = ArgList()
    << Arg("artist", artist.toLower())
    << Arg("release_title", album.toLower())
    << Arg("type", "master");

  EncodedArgList encoded_args;

  foreach (const Arg& arg, args) {
    EncodedArg encoded_arg(QUrl::toPercentEncoding(arg.first),
                           QUrl::toPercentEncoding(arg.second));
    encoded_args << encoded_arg;
  }

  url.setEncodedQueryItems(encoded_args);

  QNetworkReply* reply = network_->get(QNetworkRequest(url));
  NewClosure(reply, SIGNAL(finished()),
             this, SLOT(SearchReply(QNetworkReply*, int)),
             reply, id);

  return true;
}

// Parse the reply from a search.  
void DiscogsCoverProvider::SearchReply(QNetworkReply* reply, int id) {
  reply->deleteLater();

  QJson::Parser parser;
  bool ok;
  QVariantMap reply_map = parser.parse(reply, &ok).toMap();

  if (!ok || !reply_map.contains("results")) {
    CoverSearchResults cover_results;
    emit SearchFinished(id, cover_results);
    return;
  }

  QVariantList results = reply_map["results"].toList();
  FollowBestResult(reply, id, results);
}

void DiscogsCoverProvider::FollowBestResult(QNetworkReply* reply, int id, QVariantList results) {
  
  // first pass - check for the master(s)
  foreach (const QVariant& result, results) {
    QVariantMap result_map = result.toMap();
    if (result_map.contains("type")
        && result_map["type"].toString() == "master"
        && result_map.contains("resource_url")) {
      // follow the url
      qLog(Debug) << "Discogs: master resoure_url" << result_map["resource_url"].toString();
      results.removeOne(result);
      QueryResource(result_map["resource_url"].toString(), id, results);
      return;
    }
  }

  // otherwise, no results.
  qLog(Debug) << "Discogs: no results";
  CoverSearchResults cover_results;
  emit SearchFinished(id, cover_results);
}

// Send a new HTTP request for the selected resource.
void DiscogsCoverProvider::QueryResource(const QString& resource_url, int id, const QVariantList& results) {
  QUrl url(resource_url);
  
  QNetworkReply* reply = network_->get(QNetworkRequest(url));
  NewClosure(reply, SIGNAL(finished()),
             this, SLOT(QueryResourceReply(QNetworkReply*, int, const QVariantList&)),
             reply, id, results);
}

// Parse the resource, look for images.
void DiscogsCoverProvider::QueryResourceReply(QNetworkReply* reply, int id, const QVariantList& results) {
  reply->deleteLater();

  QJson::Parser parser;
  bool ok;
  bool primary_found = false;
  QVariantMap reply_map = parser.parse(reply, &ok).toMap();

  if (!ok || !reply_map.contains("images")) {
    // try with the next result, if any
    qLog(Debug) << "Discogs: no images found, trying with next result";
    FollowBestResult(reply, id, results);
    return;
  }

  CoverSearchResults cover_results;

  QVariantList images = reply_map["images"].toList();

  // first pass, scan for primary images (there will typically be exactly one)
  foreach (const QVariant& image, images) {
    QVariantMap image_map = image.toMap();
    if (image_map.contains("type")
        && image_map["type"].toString() == "primary"
        && image_map.contains("uri")) {
      CoverSearchResult cover_result;
      cover_result.image_url = image_map["uri"].toString();
      qLog(Debug) << "Discogs: Found primary image" << cover_result.image_url;
      cover_results.append(cover_result);
      primary_found = true;
    }
  }

  if (primary_found) {
    emit SearchFinished(id, cover_results);
    return;
  }
  
  // second pass, take what we've got (odd case - only secondary images)
  foreach (const QVariant& image, images) {
    QVariantMap image_map = image.toMap();
    if (image_map.contains("uri")) {
      CoverSearchResult cover_result;
      cover_result.image_url = image_map["uri"].toString();
      qLog(Debug) << "Discogs: Found other image" << cover_result.image_url;
      cover_results.append(cover_result);
    }
  }
  emit SearchFinished(id, cover_results);
}
