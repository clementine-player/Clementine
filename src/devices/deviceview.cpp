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

#include "connecteddevice.h"
#include "devicemanager.h"
#include "deviceview.h"
#include "core/mergedproxymodel.h"
#include "library/librarymodel.h"
#include "ui/iconloader.h"

#include <QContextMenuEvent>
#include <QMenu>
#include <QSortFilterProxyModel>

#include <boost/shared_ptr.hpp>

DeviceView::DeviceView(QWidget* parent)
  : QTreeView(parent),
    manager_(NULL),
    merged_model_(NULL),
    sort_model_(NULL),
    menu_(new QMenu(this))
{
  connect_action_ = menu_->addAction(
      IconLoader::Load("list-add"), tr("Connect device"), this, SLOT(Connect()));
  disconnect_action_ = menu_->addAction(
      IconLoader::Load("list-remove"), tr("Disconnect device"), this, SLOT(Disconnect()));
}

void DeviceView::SetDeviceManager(DeviceManager *manager) {
  Q_ASSERT(manager_ == NULL);

  manager_ = manager;
  connect(manager_, SIGNAL(DeviceDisconnected(int)), SLOT(DeviceDisconnected(int)));

  merged_model_ = new MergedProxyModel(this);
  merged_model_->setSourceModel(manager_);

  sort_model_ = new QSortFilterProxyModel(this);
  sort_model_->setSourceModel(merged_model_);
  sort_model_->setDynamicSortFilter(true);
  sort_model_->sort(0);

  setModel(sort_model_);
}

void DeviceView::contextMenuEvent(QContextMenuEvent* e) {
  menu_index_ = currentIndex();
  QModelIndex device_index = MapToDevice(menu_index_);
  bool is_device = device_index.isValid();
  bool is_connected = is_device && manager_->GetConnectedDevice(device_index.row());

  connect_action_->setEnabled(is_device);
  disconnect_action_->setEnabled(is_device);
  connect_action_->setVisible(!is_connected);
  disconnect_action_->setVisible(is_connected);

  menu_->popup(e->globalPos());
}

QModelIndex DeviceView::MapToDevice(const QModelIndex &sort_model_index) const {
  if (sort_model_index.model() != sort_model_)
    return QModelIndex();

  QModelIndex index =
      merged_model_->mapToSource(sort_model_->mapToSource(sort_model_index));
  if (index.model() != manager_)
    return QModelIndex();
  return index;
}

void DeviceView::Connect() {
  QModelIndex device_idx = MapToDevice(menu_index_);

  boost::shared_ptr<ConnectedDevice> device = manager_->Connect(device_idx.row());
  merged_model_->AddSubModel(device_idx, device->model());
}

void DeviceView::Disconnect() {
  QModelIndex device_idx = MapToDevice(menu_index_);

  manager_->Disconnect(device_idx.row());
}

void DeviceView::DeviceDisconnected(int row) {
  merged_model_->RemoveSubModel(manager_->index(row));
}
