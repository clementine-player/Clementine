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

#ifndef COVERS_DISCOGSCOVERPROVIDER_H
#define COVERS_DISCOGSCOVERPROVIDER_H

#include <QNetworkReply>
#include <QXmlStreamReader>

#include "coverprovider.h"

class QNetworkAccessManager;

// This struct represents a single search request. It identifies and describes
// the request.
struct DiscogsCoverSearchContext {
  enum State { State_Init, State_Release };

  // the unique request identifier
  int id;

  // the search query
  QString artist;
  QString album;
  QString title;
  int r_count;

  CoverSearchResults results;
};
Q_DECLARE_METATYPE(DiscogsCoverSearchContext)

// This struct represents a single release request. It identifies and describes
// the request.
struct DiscogsCoverReleaseContext {
  int id;    // the unique request identifier
  int s_id;  // the search request identifier

  QString resource_url;
};
Q_DECLARE_METATYPE(DiscogsCoverReleaseContext)

class DiscogsCoverProvider : public CoverProvider {
  Q_OBJECT

 public:
  explicit DiscogsCoverProvider(QObject* parent = nullptr);

  static const char* kUrlSearch;
  static const char* kUrlReleases;

  static const char* kAccessKeyB64;
  static const char* kSecretKeyB64;

  bool StartSearch(const QString& artist, const QString& album, int s_id);
  void CancelSearch(int id);

 private slots:
  void SearchRequestError(QNetworkReply::NetworkError error,
                          QNetworkReply* reply, int s_id);
  void ReleaseRequestError(QNetworkReply::NetworkError error,
                           QNetworkReply* reply, int s_id, int r_id);
  void HandleSearchReply(QNetworkReply* reply, int s_id);
  void HandleReleaseReply(QNetworkReply* reply, int sa_id, int si_id);

 private:
  QNetworkAccessManager* network_;
  QHash<int, DiscogsCoverSearchContext*> requests_search_;
  QHash<int, DiscogsCoverReleaseContext*> requests_release_;

  bool StartRelease(DiscogsCoverSearchContext* s_ctx, int r_id,
                    QString resource_url);

  void SendSearchRequest(DiscogsCoverSearchContext* s_ctx);
  void SendReleaseRequest(DiscogsCoverSearchContext* s_ctx,
                          DiscogsCoverReleaseContext* r_ctx);
  void EndSearch(DiscogsCoverSearchContext* s_ctx,
                 DiscogsCoverReleaseContext* r_ctx);
  void EndSearch(DiscogsCoverSearchContext* s_ctx);
  void EndSearch(DiscogsCoverReleaseContext* r_ctx);
};

#endif  // COVERS_DISCOGSCOVERPROVIDER_H
