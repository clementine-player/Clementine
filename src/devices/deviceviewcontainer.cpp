/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>

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

#include "deviceviewcontainer.h"
#include "ui_deviceviewcontainer.h"
#include "ui/iconloader.h"

DeviceViewContainer::DeviceViewContainer(QWidget* parent)
    : QWidget(parent), ui_(new Ui::DeviceViewContainer), loaded_icons_(false) {
  ui_->setupUi(this);

  QPalette palette(ui_->windows_is_broken_frame->palette());
  palette.setColor(QPalette::Background, QColor(255, 255, 222));
  ui_->windows_is_broken_frame->setPalette(palette);

#ifdef Q_OS_WIN
  ui_->windows_is_broken_frame->show();
#else
  ui_->windows_is_broken_frame->hide();
#endif
}

DeviceViewContainer::~DeviceViewContainer() { delete ui_; }

void DeviceViewContainer::showEvent(QShowEvent* e) {
  if (!loaded_icons_) {
    loaded_icons_ = true;

    ui_->close_frame_button->setIcon(IconLoader::Load("edit-delete"));
    ui_->warning_icon->setPixmap(IconLoader::Load("dialog-warning").pixmap(22));
  }

  QWidget::showEvent(e);
}

DeviceView* DeviceViewContainer::view() const { return ui_->view; }
