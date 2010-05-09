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

#ifndef SAVEDRADIO_H
#define SAVEDRADIO_H

#include "radioservice.h"

class QMenu;

class SavedRadio : public RadioService {
  Q_OBJECT

 public:
  SavedRadio(RadioModel* parent);
  ~SavedRadio();

  enum ItemType {
    Type_Stream = 2000,
  };

  static const char* kServiceName;
  static const char* kSettingsGroup;

  RadioItem* CreateRootItem(RadioItem* parent);
  void LazyPopulate(RadioItem* item);

  void ShowContextMenu(RadioItem* item, const QModelIndex& index,
                       const QPoint& global_pos);

  void StartLoading(const QUrl& url);

  void Add(const QUrl& url);

 signals:
  void ShowAddStreamDialog();

 private slots:
  void AddToPlaylist();
  void Remove();

 private:
  void LoadStreams();
  void SaveStreams();
  RadioItem* ItemForStream(const QUrl& url, RadioItem* parent);

 private:
  RadioItem* root_;
  QMenu* context_menu_;
  RadioItem* context_item_;

  QAction* add_action_;
  QAction* remove_action_;

  QStringList streams_;
};

#endif // SAVEDRADIO_H
