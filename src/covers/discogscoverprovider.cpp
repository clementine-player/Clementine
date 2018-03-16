/* This file is part of Clementine.
   Copyright 2012, Martin Björklund <mbj4668@gmail.com>
   Copyright 2018, Jonas Kvinge <jonas@jkvinge.net>

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

#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QList>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPair>
#include <QString>
#include <QStringList>
#include <QUrlQuery>
#include <QVariant>

#include "discogscoverprovider.h"

#include "core/closure.h"
#include "core/logging.h"
#include "core/network.h"
#include "core/utilities.h"

const char* DiscogsCoverProvider::kUrlSearch =
    "https://api.discogs.com/database/search";
const char* DiscogsCoverProvider::kUrlReleases =
    "https://api.discogs.com/releases";

const char* DiscogsCoverProvider::kAccessKeyB64 =
    "YVR4Yk5JTnlmUkhFY0pTaldid2c=";
const char* DiscogsCoverProvider::kSecretKeyB64 =
    "QkJNb2tMVXVUVFhSRWRUVmZDc0ZGamZmSWRjdHZRVno=";

DiscogsCoverProvider::DiscogsCoverProvider(QObject* parent)
    : CoverProvider("Discogs", false, parent),
      network_(new NetworkAccessManager(this)) {}

bool DiscogsCoverProvider::StartSearch(const QString& artist,
                                       const QString& album, int s_id) {
  DiscogsCoverSearchContext* s_ctx = new DiscogsCoverSearchContext;

  s_ctx->id = s_id;
  s_ctx->artist = artist;
  s_ctx->album = album;
  s_ctx->r_count = 0;
  requests_search_.insert(s_id, s_ctx);
  SendSearchRequest(s_ctx);

  return true;
}

void DiscogsCoverProvider::CancelSearch(int id) {
  delete requests_search_.take(id);
}

bool DiscogsCoverProvider::StartRelease(DiscogsCoverSearchContext* s_ctx,
                                        int r_id, QString resource_url) {
  DiscogsCoverReleaseContext* r_ctx = new DiscogsCoverReleaseContext;

  s_ctx->r_count++;

  r_ctx->id = r_id;
  r_ctx->resource_url = resource_url;

  r_ctx->s_id = s_ctx->id;

  requests_release_.insert(r_id, r_ctx);
  SendReleaseRequest(s_ctx, r_ctx);

  return true;
}

void DiscogsCoverProvider::SendSearchRequest(DiscogsCoverSearchContext* s_ctx) {
  typedef QPair<QString, QString> Arg;
  typedef QList<Arg> ArgList;

  typedef QPair<QByteArray, QByteArray> EncodedArg;
  typedef QList<EncodedArg> EncodedArgList;

  ArgList args =
      ArgList() << Arg("key", QByteArray::fromBase64(kAccessKeyB64))
                << Arg("secret", QByteArray::fromBase64(kSecretKeyB64));

  args.append(Arg("type", "release"));
  if (!s_ctx->artist.isEmpty()) {
    args.append(Arg("artist", s_ctx->artist.toLower()));
  }
  if (!s_ctx->album.isEmpty()) {
    args.append(Arg("release_title", s_ctx->album.toLower()));
  }

  QUrlQuery url_query;
  QUrl url(kUrlSearch);
  QStringList query_items;

  // Encode the arguments
  for (const Arg& arg : args) {
    EncodedArg encoded_arg(QUrl::toPercentEncoding(arg.first),
                           QUrl::toPercentEncoding(arg.second));
    query_items << QString(encoded_arg.first + "=" + encoded_arg.second);
    url_query.addQueryItem(encoded_arg.first, encoded_arg.second);
  }

  // Sign the request
  const QByteArray data_to_sign =
      QString("GET\n%1\n%2\n%3")
          .arg(url.host(), url.path(), query_items.join("&"))
          .toUtf8();
  const QByteArray signature(Utilities::HmacSha256(
      QByteArray::fromBase64(kSecretKeyB64), data_to_sign));

  // Add the signature to the request
  url_query.addQueryItem("Signature",
                         QUrl::toPercentEncoding(signature.toBase64()));

  url.setQuery(url_query);
  QNetworkReply* reply = network_->get(QNetworkRequest(url));

  NewClosure(reply, SIGNAL(error(QNetworkReply::NetworkError)), this,
             SLOT(SearchRequestError(QNetworkReply::NetworkError,
                                     QNetworkReply*, int)),
             reply, s_ctx->id);
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(HandleSearchReply(QNetworkReply*, int)), reply, s_ctx->id);
}

void DiscogsCoverProvider::SendReleaseRequest(
    DiscogsCoverSearchContext* s_ctx, DiscogsCoverReleaseContext* r_ctx) {
  typedef QPair<QString, QString> Arg;
  typedef QList<Arg> ArgList;

  typedef QPair<QByteArray, QByteArray> EncodedArg;
  typedef QList<EncodedArg> EncodedArgList;

  QUrlQuery url_query;
  QStringList query_items;

  ArgList args =
      ArgList() << Arg("key", QByteArray::fromBase64(kAccessKeyB64))
                << Arg("secret", QByteArray::fromBase64(kSecretKeyB64));
  // Encode the arguments
  for (const Arg& arg : args) {
    EncodedArg encoded_arg(QUrl::toPercentEncoding(arg.first),
                           QUrl::toPercentEncoding(arg.second));
    query_items << QString(encoded_arg.first + "=" + encoded_arg.second);
    url_query.addQueryItem(encoded_arg.first, encoded_arg.second);
  }

  QUrl url(r_ctx->resource_url);

  // Sign the request
  const QByteArray data_to_sign =
      QString("GET\n%1\n%2\n%3")
          .arg(url.host(), url.path(), query_items.join("&"))
          .toUtf8();
  const QByteArray signature(Utilities::HmacSha256(
      QByteArray::fromBase64(kSecretKeyB64), data_to_sign));

  // Add the signature to the request
  url_query.addQueryItem("Signature",
                         QUrl::toPercentEncoding(signature.toBase64()));

  url.setQuery(url_query);
  QNetworkReply* reply = network_->get(QNetworkRequest(url));

  NewClosure(reply, SIGNAL(error(QNetworkReply::NetworkError)), this,
             SLOT(ReleaseRequestError(QNetworkReply::NetworkError,
                                      QNetworkReply*, int, int)),
             reply, s_ctx->id, r_ctx->id);
  NewClosure(reply, SIGNAL(finished()), this,
             SLOT(HandleReleaseReply(QNetworkReply*, int, int)), reply,
             s_ctx->id, r_ctx->id);
}

void DiscogsCoverProvider::HandleSearchReply(QNetworkReply* reply, int s_id) {
  reply->deleteLater();

  if (!requests_search_.contains(s_id)) return;
  DiscogsCoverSearchContext* s_ctx = requests_search_.value(s_id);

  QJsonDocument json_doc = QJsonDocument::fromJson(reply->readAll());
  if ((json_doc.isNull()) || (!json_doc.isObject())) {
    qLog(Error) << "Discogs: Failed to create JSON doc.";
    EndSearch(s_ctx);
    return;
  }

  QJsonObject json_obj = json_doc.object();
  if (json_obj.isEmpty()) {
    qLog(Error) << "Discogs: Failed to create JSON object.";
    EndSearch(s_ctx);
    return;
  }

  QVariantMap reply_map = json_obj.toVariantMap();
  if (!reply_map.contains("results")) {
    EndSearch(s_ctx);
    return;
  }

  QVariantList results = reply_map["results"].toList();
  int i = 0;
  for (const QVariant& result : results) {
    QVariantMap result_map = result.toMap();
    if ((result_map.contains("id")) && (result_map.contains("resource_url"))) {
      int r_id = result_map["id"].toInt();
      QString title = result_map["title"].toString();
      QString resource_url = result_map["resource_url"].toString();
      if (resource_url.isEmpty()) continue;
      StartRelease(s_ctx, r_id, resource_url);
      i++;
    }
  }
  if (i <= 0) EndSearch(s_ctx);
}

void DiscogsCoverProvider::HandleReleaseReply(QNetworkReply* reply, int s_id,
                                              int r_id) {
  reply->deleteLater();

  if (!requests_release_.contains(r_id)) return;
  DiscogsCoverReleaseContext* r_ctx = requests_release_.value(r_id);

  if (!requests_search_.contains(s_id)) {
    EndSearch(r_ctx);
    return;
  }
  DiscogsCoverSearchContext* s_ctx = requests_search_.value(s_id);

  QJsonDocument json_doc = QJsonDocument::fromJson(reply->readAll());
  if ((json_doc.isNull()) || (!json_doc.isObject())) {
    qLog(Error) << "Discogs: Failed to create JSON doc.";
    EndSearch(s_ctx, r_ctx);
    return;
  }
  QJsonObject json_obj = json_doc.object();
  if (json_obj.isEmpty()) {
    qLog(Error) << "Discogs: JSON object is empty.";
    EndSearch(s_ctx, r_ctx);
    return;
  }

  QVariantMap reply_map = json_obj.toVariantMap();
  if (!reply_map.contains("images")) {
    EndSearch(s_ctx, r_ctx);
    return;
  }

  QVariantList results = reply_map["images"].toList();

  for (const QVariant& result : results) {
    QVariantMap result_map = result.toMap();
    CoverSearchResult cover_result;
    cover_result.description = s_ctx->title;

    if (result_map.contains("type")) {
      QString type = result_map["type"].toString();
      if (type != "primary") continue;
    }
    if (result_map.contains("resource_url")) {
      cover_result.image_url = QUrl(result_map["resource_url"].toString());
    }
    if (cover_result.image_url.isEmpty()) continue;
    s_ctx->results.append(cover_result);
  }

  EndSearch(s_ctx, r_ctx);
}

void DiscogsCoverProvider::SearchRequestError(QNetworkReply::NetworkError error,
                                              QNetworkReply* reply, int s_id) {
  if (!requests_search_.contains(s_id)) return;
  DiscogsCoverSearchContext* s_ctx = requests_search_.value(s_id);

  EndSearch(s_ctx);
}

void DiscogsCoverProvider::ReleaseRequestError(
    QNetworkReply::NetworkError error, QNetworkReply* reply, int s_id,
    int r_id) {

  if (!requests_release_.contains(r_id)) return;
  DiscogsCoverReleaseContext* r_ctx = requests_release_.value(r_id);

  if (!requests_search_.contains(s_id)) {
    EndSearch(r_ctx);
    return;
  }
  DiscogsCoverSearchContext* s_ctx = requests_search_.value(s_id);
  EndSearch(s_ctx, r_ctx);
}

void DiscogsCoverProvider::EndSearch(DiscogsCoverSearchContext* s_ctx,
                                     DiscogsCoverReleaseContext* r_ctx) {
  delete requests_release_.take(r_ctx->id);

  s_ctx->r_count--;

  if (s_ctx->r_count <= 0) EndSearch(s_ctx);
}

void DiscogsCoverProvider::EndSearch(DiscogsCoverSearchContext* s_ctx) {
  requests_search_.remove(s_ctx->id);
  emit SearchFinished(s_ctx->id, s_ctx->results);
  delete s_ctx;
}

void DiscogsCoverProvider::EndSearch(DiscogsCoverReleaseContext* r_ctx) {
  delete requests_release_.take(r_ctx->id);
}
