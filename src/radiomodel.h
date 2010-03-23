/* This file is part of Clementine.

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

#ifndef RADIOMODEL_H
#define RADIOMODEL_H

#include "radioitem.h"
#include "simpletreemodel.h"
#include "multiloadingindicator.h"

class RadioService;
class LastFMService;
class Song;

class RadioModel : public SimpleTreeModel<RadioItem> {
  Q_OBJECT

 public:
  RadioModel(QObject* parent = 0);

  enum {
    Role_Type = Qt::UserRole + 1,
    Role_SortText,
    Role_Key,
  };

  // Needs to be static for RadioPlaylistItem::restore
  static RadioService* ServiceByName(const QString& name);

  // This is special because Player needs it for scrobbling
  LastFMService* GetLastFMService() const;

  // QAbstractItemModel
  QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
  Qt::ItemFlags flags(const QModelIndex& index) const;
  QStringList mimeTypes() const;
  QMimeData* mimeData(const QModelIndexList& indexes) const;

  void ShowContextMenu(RadioItem* item, const QPoint& global_pos);
  void ReloadSettings();

 signals:
  void TaskStarted(MultiLoadingIndicator::TaskType);
  void TaskFinished(MultiLoadingIndicator::TaskType);
  void StreamReady(const QUrl& original_url, const QUrl& media_url);
  void StreamFinished();
  void StreamError(const QString& message);
  void StreamMetadataFound(const QUrl& original_url, const Song& song);

  void AddItemToPlaylist(RadioItem* item);

 protected:
  void LazyPopulate(RadioItem* parent);

 private:
  QVariant data(const RadioItem* item, int role) const;
  void AddService(RadioService* service);

 private:
  static QMap<QString, RadioService*> sServices;
};

#endif // RADIOMODEL_H
