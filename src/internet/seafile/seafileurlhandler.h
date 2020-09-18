/* This file is part of Clementine.
   Copyright 2014, Chocobozzz <djidane14ff@hotmail.fr>
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

#ifndef INTERNET_SEAFILE_SEAFILEURLHANDLER_H_
#define INTERNET_SEAFILE_SEAFILEURLHANDLER_H_

#include "core/urlhandler.h"
#include "ui/iconloader.h"

class SeafileService;

class SeafileUrlHandler : public UrlHandler {
  Q_OBJECT
 public:
  explicit SeafileUrlHandler(SeafileService* service,
                             QObject* parent = nullptr);

  QString scheme() const { return "seafile"; }
  QIcon icon() const {
    return IconLoader::Load("seafile", IconLoader::Provider);
  }
  LoadResult StartLoading(const QUrl& url);

 private:
  SeafileService* service_;
};

#endif  // INTERNET_SEAFILE_SEAFILEURLHANDLER_H_
