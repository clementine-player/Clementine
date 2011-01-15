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

#ifndef RADIOPLAYLISTITEM_H
#define RADIOPLAYLISTITEM_H

#include "core/song.h"
#include "playlist/playlistitem.h"

#include <QUrl>

class RadioService;

class RadioPlaylistItem : public PlaylistItem {
 public:
  RadioPlaylistItem(const QString& type);
  RadioPlaylistItem(RadioService* service, const QUrl& url,
                    const QString& title, const QString& artist);

  Options options() const;

  bool InitFromQuery(const SqlRow& query);
  void BindToQuery(QSqlQuery *query) const;

  Song Metadata() const;
  QUrl Url() const;

  SpecialLoadResult StartLoading();
  SpecialLoadResult LoadNext();

 protected:
  QVariant DatabaseValue(DatabaseColumn) const;

 private:
  void InitMetadata();
  RadioService* service() const;

 private:
  QUrl url_;
  QString title_;
  QString artist_;
  QString service_name_;

  bool set_service_icon_;

  Song metadata_;
};

#endif // RADIOPLAYLISTITEM_H
