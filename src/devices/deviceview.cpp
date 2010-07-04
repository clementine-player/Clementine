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
#include "deviceproperties.h"
#include "deviceview.h"
#include "core/mergedproxymodel.h"
#include "library/librarymodel.h"
#include "ui/iconloader.h"

#include <QContextMenuEvent>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QSortFilterProxyModel>

#include <boost/shared_ptr.hpp>

const int DeviceItemDelegate::kIconPadding = 6;

DeviceItemDelegate::DeviceItemDelegate(QObject *parent)
  : LibraryItemDelegate(parent)
{
}

void DeviceItemDelegate::paint(QPainter* p, const QStyleOptionViewItem& opt, const QModelIndex& index) const {
  // Is it a device or a library item?
  if (index.data(DeviceManager::Role_State).isNull()) {
    LibraryItemDelegate::paint(p, opt, index);
    return;
  }

  // Draw the background
  QStyledItemDelegate::paint(p, opt, QModelIndex());

  p->save();

  // Font for the status line
  QFont status_font(opt.font);
  status_font.setItalic(true);
  status_font.setPointSize(status_font.pointSize() - 2);

  const int text_height = QFontMetrics(opt.font).height() +
                          QFontMetrics(status_font).height();

  QRect line1(opt.rect);
  QRect line2(opt.rect);
  line1.setTop(line1.top() + (opt.rect.height() - text_height) / 2);
  line2.setTop(line1.top() + QFontMetrics(opt.font).height());
  line1.setLeft(line1.left() + DeviceManager::kDeviceIconSize + kIconPadding);
  line2.setLeft(line2.left() + DeviceManager::kDeviceIconSize + kIconPadding);

  // Change the color for selected items
  if (opt.state & QStyle::State_Selected) {
    p->setPen(opt.palette.color(QPalette::HighlightedText));
  }

  // Draw the icon
  p->drawPixmap(opt.rect.topLeft(), index.data(Qt::DecorationRole).value<QPixmap>());

  // Draw the first line (device name)
  p->drawText(line1, Qt::AlignLeft | Qt::AlignTop, index.data().toString());

  // Draw the second line (status)
  p->setFont(status_font);

  DeviceManager::State state =
      static_cast<DeviceManager::State>(index.data(DeviceManager::Role_State).toInt());
  QVariant progress = index.data(DeviceManager::Role_UpdatingPercentage);
  QString status_text;

  if (progress.isValid()) {
    status_text = tr("Updating %1%...").arg(progress.toInt());
  } else {
    switch (state) {
      case DeviceManager::State_Remembered:
        status_text = tr("Not connected");
        break;

      case DeviceManager::State_NotConnected:
        status_text = tr("Double click to open");
        break;

      case DeviceManager::State_Connected:
        status_text = tr("Connected");
        break;
    }
  }

  p->drawText(line2, Qt::AlignLeft | Qt::AlignTop, status_text);

  p->restore();
}



DeviceView::DeviceView(QWidget* parent)
  : AutoExpandingTreeView(parent),
    manager_(NULL),
    merged_model_(NULL),
    sort_model_(NULL),
    properties_dialog_(new DeviceProperties),
    menu_(new QMenu(this))
{
  connect_action_ = menu_->addAction(
      IconLoader::Load("list-add"), tr("Connect device"), this, SLOT(Connect()));
  disconnect_action_ = menu_->addAction(
      IconLoader::Load("list-remove"), tr("Disconnect device"), this, SLOT(Disconnect()));
  forget_action_ = menu_->addAction(
      IconLoader::Load("list-remove"), tr("Forget device"), this, SLOT(Forget()));
  menu_->addSeparator();
  properties_action_ = menu_->addAction(
      IconLoader::Load("configure"), tr("Device properties..."), this, SLOT(Properties()));

  setItemDelegate(new DeviceItemDelegate(this));
  SetExpandOnReset(false);
}

DeviceView::~DeviceView() {
}

void DeviceView::SetDeviceManager(DeviceManager *manager) {
  Q_ASSERT(manager_ == NULL);

  manager_ = manager;
  connect(manager_, SIGNAL(DeviceDisconnected(int)), SLOT(DeviceDisconnected(int)));

  sort_model_ = new QSortFilterProxyModel(this);
  sort_model_->setSourceModel(manager_);
  sort_model_->setDynamicSortFilter(true);
  sort_model_->sort(0);

  merged_model_ = new MergedProxyModel(this);
  merged_model_->setSourceModel(sort_model_);

  connect(merged_model_,
          SIGNAL(SubModelReset(QModelIndex,QAbstractItemModel*)),
          SLOT(RecursivelyExpand(QModelIndex)));

  setModel(merged_model_);
  properties_dialog_->SetDeviceManager(manager_);
}

void DeviceView::contextMenuEvent(QContextMenuEvent* e) {
  menu_index_ = currentIndex();
  QModelIndex device_index = MapToDevice(menu_index_);
  bool is_device = device_index.isValid();
  bool is_connected = is_device && manager_->GetConnectedDevice(device_index.row());
  bool is_plugged_in = is_device && manager_->GetLister(device_index.row());
  bool is_remembered = is_device && manager_->GetDatabaseId(device_index.row()) != -1;

  connect_action_->setEnabled(is_plugged_in);
  disconnect_action_->setEnabled(is_plugged_in);
  connect_action_->setVisible(!is_connected);
  disconnect_action_->setVisible(is_connected);
  forget_action_->setEnabled(is_remembered);

  menu_->popup(e->globalPos());
}

QModelIndex DeviceView::MapToDevice(const QModelIndex& merged_model_index) const {
  QModelIndex sort_model_index = merged_model_->mapToSource(merged_model_index);
  if (sort_model_index.model() != sort_model_)
    return QModelIndex();

  return sort_model_->mapToSource(sort_model_index);
}

void DeviceView::Connect() {
  QModelIndex device_idx = MapToDevice(menu_index_);
  QModelIndex sort_idx = sort_model_->mapFromSource(device_idx);
  bool first_time = manager_->GetDatabaseId(device_idx.row()) == -1;

  if (first_time) {
    boost::scoped_ptr<QMessageBox> dialog(new QMessageBox(
        QMessageBox::Information, tr("Connect device"),
        tr("This is the first time you have connected this device.  Clementine will now scan the device to find music files - this may take some time."),
        QMessageBox::Cancel, this));
    QPushButton* connect =
        dialog->addButton(tr("Connect device"), QMessageBox::AcceptRole);
    dialog->exec();

    if (dialog->clickedButton() != connect)
      return;
  }

  boost::shared_ptr<ConnectedDevice> device = manager_->Connect(device_idx.row());
  if (!device)
    return;

  QSortFilterProxyModel* sort_model = new QSortFilterProxyModel(device->model());
  sort_model->setSourceModel(device->model());
  sort_model->setSortRole(LibraryModel::Role_SortText);
  sort_model->setDynamicSortFilter(true);
  sort_model->sort(0);
  merged_model_->AddSubModel(sort_idx, sort_model);

  expand(menu_index_);
}

void DeviceView::Disconnect() {
  QModelIndex device_idx = MapToDevice(menu_index_);
  manager_->Disconnect(device_idx.row());
}

void DeviceView::DeviceDisconnected(int row) {
  merged_model_->RemoveSubModel(sort_model_->mapFromSource(manager_->index(row)));
}

void DeviceView::Forget() {
  boost::scoped_ptr<QMessageBox> dialog(new QMessageBox(
      QMessageBox::Question, tr("Forget device"),
      tr("Forgetting a device will remove it from this list and Clementine will have to rescan all the songs again next time you connect it."),
      QMessageBox::Cancel, this));
  QPushButton* forget =
      dialog->addButton(tr("Forget device"), QMessageBox::DestructiveRole);
  dialog->exec();

  if (dialog->clickedButton() != forget)
    return;

  QModelIndex device_idx = MapToDevice(menu_index_);
  manager_->Forget(device_idx.row());
}

void DeviceView::Properties() {
  properties_dialog_->ShowDevice(MapToDevice(menu_index_).row());
}

void DeviceView::mouseDoubleClickEvent(QMouseEvent *event) {
  AutoExpandingTreeView::mouseDoubleClickEvent(event);

  QModelIndex merged_index = indexAt(event->pos());
  QModelIndex device_index = MapToDevice(merged_index);
  if (device_index.isValid() && !manager_->GetConnectedDevice(device_index.row())) {
    menu_index_ = merged_index;
    Connect();
  }
}
