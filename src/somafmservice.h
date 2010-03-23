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

#ifndef SOMAFMSERVICE_H
#define SOMAFMSERVICE_H

#include <QXmlStreamReader>

#include "radioservice.h"

class QNetworkAccessManager;
class QMenu;

class SomaFMService : public RadioService {
  Q_OBJECT

 public:
  SomaFMService(QObject* parent = 0);
  ~SomaFMService();

  enum ItemType {
    Type_Stream = 2000,
  };

  static const char* kServiceName;
  static const char* kChannelListUrl;
  static const char* kHomepage;

  RadioItem* CreateRootItem(RadioItem* parent);
  void LazyPopulate(RadioItem* item);

  QString TitleForItem(const RadioItem* item) const;

  void ShowContextMenu(RadioItem* item, const QPoint& global_pos);

  void StartLoading(const QUrl& url);

 private slots:
  void RefreshChannels();
  void RefreshChannelsFinished();
  void LoadPlaylistFinished();

  void AddToPlaylist();
  void Homepage();

 private:
  void ReadChannel(QXmlStreamReader& reader);
  void ConsumeElement(QXmlStreamReader& reader);

 private:
  RadioItem* root_;
  QMenu* context_menu_;
  RadioItem* context_item_;

  QNetworkAccessManager* network_;
};

#endif // SOMAFMSERVICE_H
