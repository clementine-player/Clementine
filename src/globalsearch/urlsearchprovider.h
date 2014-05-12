/* This file is part of Clementine.
   Copyright 2011, David Sansome <me@davidsansome.com>

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

#ifndef URLSEARCHPROVIDER_H
#define URLSEARCHPROVIDER_H

#include "searchprovider.h"

#include <QRegExp>

class UrlSearchProvider : public SearchProvider {
 public:
  UrlSearchProvider(Application* app, QObject* parent);

  bool LooksLikeUrl(const QString& query) const;

  void SearchAsync(int id, const QString& query);
  void LoadArtAsync(int id, const Result& result);

 private:
  static const char* kUrlRegex;
  QRegExp url_regex_;

  QImage image_;
};

#endif  // URLSEARCHPROVIDER_H
