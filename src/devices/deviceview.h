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

#ifndef DEVICEVIEW_H
#define DEVICEVIEW_H

#include <QTreeView>

class QAction;
class QMenu;
class QSortFilterProxyModel;

class DeviceManager;
class MergedProxyModel;

class DeviceView : public QTreeView {
  Q_OBJECT

public:
  DeviceView(QWidget* parent = 0);

  void SetDeviceManager(DeviceManager* manager);

protected:
  void contextMenuEvent(QContextMenuEvent *);

private slots:
  void Connect();
  void Disconnect();

  void DeviceDisconnected(int row);

private:
  QModelIndex MapToDevice(const QModelIndex& sort_model_index) const;

private:
  DeviceManager* manager_;
  MergedProxyModel* merged_model_;
  QSortFilterProxyModel* sort_model_;

  QMenu* menu_;
  QAction* connect_action_;
  QAction* disconnect_action_;
  QModelIndex menu_index_;
};

#endif // DEVICEVIEW_H
