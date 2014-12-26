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

#ifndef COVERS_DISCOGSCOVERPROVIDER_H_
#define COVERS_DISCOGSCOVERPROVIDER_H_

#include "coverprovider.h"
#include <QVariant>

class QNetworkAccessManager;

// This struct represents a single search-for-cover request. It identifies
// and describes the request.
struct DiscogsCoverSearchContext {
  enum State { State_Init, State_MastersRequested, State_ReleasesRequested };

  // the unique request identifier
  int id;

  // the search query
  QString artist;
  QString album;

  State state;

  CoverSearchResults results;
};
Q_DECLARE_METATYPE(DiscogsCoverSearchContext)

class DiscogsCoverProvider : public CoverProvider {
  Q_OBJECT

 public:
  explicit DiscogsCoverProvider(QObject* parent = nullptr);

  static const char* kSearchUrl;

  bool StartSearch(const QString& artist, const QString& album, int id);
  void CancelSearch(int id);

 private slots:
  void HandleSearchReply(QNetworkReply* reply, int id);

 private:
  QNetworkAccessManager* network_;
  QHash<int, DiscogsCoverSearchContext*> pending_requests_;

  void SendSearchRequest(DiscogsCoverSearchContext* ctx);
  void EndSearch(DiscogsCoverSearchContext* ctx);
};

#endif  // COVERS_DISCOGSCOVERPROVIDER_H_
