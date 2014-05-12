/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

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

#include "urlhandler.h"

UrlHandler::LoadResult::LoadResult(const QUrl& original_url, Type type,
                                   const QUrl& media_url, qint64 length_nanosec)
    : original_url_(original_url),
      type_(type),
      media_url_(media_url),
      length_nanosec_(length_nanosec) {}

UrlHandler::UrlHandler(QObject* parent) : QObject(parent) {}

QIcon UrlHandler::icon() const { return QIcon(); }
