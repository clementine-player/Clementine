/* This file is part of Clementine.
   Copyright 2012, Martin Björklund <mbj4668@gmail.com>
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

#include "discogscoverprovider.h"
#include "core/closure.h"
#include "core/logging.h"
#include "core/network.h"
#include "core/utilities.h"

#include <QNetworkReply>
#include <qjson/parser.h>

const char* DiscogsCoverProvider::kSearchUrl =
    "http://api.discogs.com/database/search";

DiscogsCoverProvider::DiscogsCoverProvider(QObject* parent)
    : CoverProvider("Discogs", parent),
      network_(new NetworkAccessManager(this)) {}

bool DiscogsCoverProvider::StartSearch(const QString& artist,
                                       const QString& album, int id) {
  DiscogsCoverSearchContext* ctx = new DiscogsCoverSearchContext;
  ctx->id = id;
  ctx->artist = artist;
  ctx->album = album;
  ctx->state = DiscogsCoverSearchContext::State_Init;
  pending_requests_.insert(id, ctx);
  SendSearchRequest(ctx);
  return true;
}

void DiscogsCoverProvider::CancelSearch(int id) {
  qLog(Debug) << "Discogs: cancel search id:" << id;
  delete pending_requests_.take(id);
}

// Send the search request using Discog's HTTP-based API.
void DiscogsCoverProvider::SendSearchRequest(DiscogsCoverSearchContext* ctx) {
  typedef QPair<QString, QString> Arg;
  typedef QList<Arg> ArgList;

  typedef QPair<QByteArray, QByteArray> EncodedArg;
  typedef QList<EncodedArg> EncodedArgList;

  QUrl url(kSearchUrl);

  QString type;

  switch (ctx->state) {
    case DiscogsCoverSearchContext::State_Init:
      type = "master";
      ctx->state = DiscogsCoverSearchContext::State_MastersRequested;
      break;
    case DiscogsCoverSearchContext::State_MastersRequested:
      type = "release";
      ctx->state = DiscogsCoverSearchContext::State_ReleasesRequested;
      break;
    default:
      EndSearch(ctx);
      return;
  }

  ArgList args = ArgList();
  if (!ctx->artist.isEmpty()) {
    args.append(Arg("artist", ctx->artist.toLower()));
  }
  if (!ctx->album.isEmpty()) {
    args.append(Arg("release_title", ctx->album.toLower()));
  }
  args.append(Arg("type", type));

  EncodedArgList encoded_args;

  for (const Arg& arg : args) {
    EncodedArg encoded_arg(QUrl::toPercentEncoding(arg.first),
                           QUrl::toPercentEncoding(arg.second));
    encoded_args << encoded_arg;
  }

  url.setEncodedQueryItems(encoded_args);

  qLog(Debug) << "Discogs: send search request for id:" << ctx->id
              << "url: " << url;

  QNetworkReply* reply = network_->get(QNetworkRequest(url));
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(HandleSearchReply(QNetworkReply*, int)), reply, ctx->id);
}

// Parse the reply from a search
void DiscogsCoverProvider::HandleSearchReply(QNetworkReply* reply, int id) {
  reply->deleteLater();

  DiscogsCoverSearchContext* ctx;
  if (!pending_requests_.contains(id)) {
    // the request was cancelled while we were waiting for the reply
    qLog(Debug) << "Discogs: got reply for cancelled request" << id;
    return;
  }
  ctx = pending_requests_.value(id);

  QJson::Parser parser;
  bool ok;
  bool found = false;
  QVariantMap reply_map = parser.parse(reply, &ok).toMap();

  if (!ok || !reply_map.contains("results")) {
    // this is an error; either parse error or bad response from the server
    EndSearch(ctx);
    return;
  }

  QVariantList results = reply_map["results"].toList();

  for (const QVariant& result : results) {
    QVariantMap result_map = result.toMap();
    // In order to use less round-trips, we cheat here.  Instead of
    // following the "resource_url", and then scan all images in the
    // resource, we go directly to the largest primary image by
    // constructing the primary image's url from the thmub's url.
    if (result_map.contains("thumb")) {
      CoverSearchResult cover_result;
      cover_result.image_url =
          QUrl(result_map["thumb"].toString().replace("R-90-", "R-"));
      if (result_map.contains("title")) {
        cover_result.description = result_map["title"].toString();
      }
      ctx->results.append(cover_result);
      found = true;
    }
  }
  if (found) {
    EndSearch(ctx);
    return;
  }

  // otherwise, no results
  switch (ctx->state) {
    case DiscogsCoverSearchContext::State_MastersRequested:
      // search again, this time for releases
      SendSearchRequest(ctx);
      break;
    default:
      EndSearch(ctx);
      break;
  }
}

void DiscogsCoverProvider::EndSearch(DiscogsCoverSearchContext* ctx) {
  (void)pending_requests_.remove(ctx->id);
  emit SearchFinished(ctx->id, ctx->results);
  delete ctx;
}
