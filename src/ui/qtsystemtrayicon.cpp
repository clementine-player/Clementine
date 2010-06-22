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

#include "qtsystemtrayicon.h"

#include <QCoreApplication>
#include <QMenu>
#include <QSystemTrayIcon>
#include <QWheelEvent>

QtSystemTrayIcon::QtSystemTrayIcon(QObject* parent)
  : SystemTrayIcon(parent),
    tray_(new QSystemTrayIcon(this)),
    menu_(new QMenu)
{
  QIcon icon(":icon.png");
  orange_icon_ = icon.pixmap(48, QIcon::Normal);
  grey_icon_ = icon.pixmap(48, QIcon::Disabled);

  tray_->setIcon(orange_icon_);
  tray_->setToolTip(QCoreApplication::applicationName());
  tray_->installEventFilter(this);

  connect(tray_, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
          SLOT(Clicked(QSystemTrayIcon::ActivationReason)));
}

QtSystemTrayIcon::~QtSystemTrayIcon() {
  delete menu_;
}

bool QtSystemTrayIcon::eventFilter(QObject* object, QEvent* event) {
  if (QObject::eventFilter(object, event))
    return true;

  if (object != tray_)
    return false;

  if (event->type() == QEvent::Wheel) {
    QWheelEvent* e = static_cast<QWheelEvent*>(event);
    emit ChangeVolume(e->delta());
    return true;
  }

  return false;
}

void QtSystemTrayIcon::SetupMenu(
    QAction* previous, QAction* play, QAction* stop, QAction* stop_after,
    QAction* next, QAction* love, QAction* ban, QAction* quit) {
  menu_->addAction(previous);
  menu_->addAction(play);
  menu_->addAction(stop);
  menu_->addAction(stop_after);
  menu_->addAction(next);
  menu_->addSeparator();
  menu_->addAction(love);
  menu_->addAction(ban);
  menu_->addSeparator();
  menu_->addAction(quit);

  tray_->setContextMenu(menu_);
}

void QtSystemTrayIcon::Clicked(QSystemTrayIcon::ActivationReason reason) {
  switch (reason) {
    case QSystemTrayIcon::DoubleClick:
    case QSystemTrayIcon::Trigger:
      emit ShowHide();
      break;

    case QSystemTrayIcon::MiddleClick:
      emit PlayPause();
      break;

    default:
      break;
  }
}

void QtSystemTrayIcon::ShowPopup(const QString &summary,
                                 const QString &message, int timeout) {
  tray_->showMessage(summary, message, QSystemTrayIcon::NoIcon, timeout);
}

void QtSystemTrayIcon::UpdateIcon() {
  tray_->setIcon(CreateIcon(orange_icon_, grey_icon_));
}

bool QtSystemTrayIcon::IsVisible() const {
  return tray_->isVisible();
}

void QtSystemTrayIcon::SetVisible(bool visible) {
  tray_->setVisible(visible);
}
