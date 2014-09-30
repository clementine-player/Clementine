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

#include "osd.h"

#include <memory>

#include <QtDebug>

#include "config.h"
#include "core/logging.h"

#ifdef HAVE_DBUS
#include "dbus/notification.h"
#include "dbus/mpris2.h"
#include <QCoreApplication>
#include <QTextDocument>

QDBusArgument& operator<<(QDBusArgument& arg, const QImage& image) {
  if (image.isNull()) {
    // Sometimes this gets called with a null QImage for no obvious reason.
    arg.beginStructure();
    arg << 0 << 0 << 0 << false << 0 << 0 << QByteArray();
    arg.endStructure();
    return arg;
  }
  QImage scaled = image.scaledToHeight(100, Qt::SmoothTransformation);

  scaled = scaled.convertToFormat(QImage::Format_ARGB32);
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
  // ABGR -> ARGB
  QImage i = scaled.rgbSwapped();
#else
  // ABGR -> GBAR
  QImage i(scaled.size(), scaled.format());
  for (int y = 0; y < i.height(); ++y) {
    QRgb* p = (QRgb*)scaled.scanLine(y);
    QRgb* q = (QRgb*)i.scanLine(y);
    QRgb* end = p + scaled.width();
    while (p < end) {
      *q = qRgba(qGreen(*p), qBlue(*p), qAlpha(*p), qRed(*p));
      p++;
      q++;
    }
  }
#endif

  arg.beginStructure();
  arg << i.width();
  arg << i.height();
  arg << i.bytesPerLine();
  arg << i.hasAlphaChannel();
  int channels = i.isGrayscale() ? 1 : (i.hasAlphaChannel() ? 4 : 3);
  arg << i.depth() / channels;
  arg << channels;
  arg << QByteArray(reinterpret_cast<const char*>(i.bits()), i.numBytes());
  arg.endStructure();
  return arg;
}

const QDBusArgument& operator>>(const QDBusArgument& arg, QImage& image) {
  // This is needed to link but shouldn't be called.
  Q_ASSERT(0);
  return arg;
}
#endif  // HAVE_DBUS

void OSD::Init() {
#ifdef HAVE_DBUS
  notification_id_ = 0;

  interface_.reset(new OrgFreedesktopNotificationsInterface(
      OrgFreedesktopNotificationsInterface::staticInterfaceName(),
      "/org/freedesktop/Notifications", QDBusConnection::sessionBus()));
  if (!interface_ || !interface_->isValid()) {
    qLog(Warning) << "Error connecting to notifications service.";
  } else {
    capabilities_ = interface_->GetCapabilities();
    connect(dynamic_cast<OrgFreedesktopNotificationsInterface*>(interface_.get()),
            SIGNAL(NotificationClosed(uint, uint)),
            this, SLOT(NotificationClosed(uint, uint)));
    connect(dynamic_cast<OrgFreedesktopNotificationsInterface*>(interface_.get()),
            SIGNAL(ActionInvoked(uint, QString)),
            this, SLOT(NotificationAction(uint, QString)));
  }
  player_.reset(new OrgMprisMediaPlayer2PlayerInterface(
      "org.mpris.MediaPlayer2.clementine",
      "/org/mpris/MediaPlayer2",
      QDBusConnection::sessionBus()));
  if (!player_ || !player_->isValid()) {
    qLog(Warning) << "Error connecting to mpris service.";
  }
#else   // HAVE_DBUS
  qLog(Warning) << "not implemented";
#endif  // HAVE_DBUS
}

bool OSD::SupportsNativeNotifications() {
#ifdef HAVE_DBUS
  return true;
#else
  return false;
#endif
}

bool OSD::SupportsTrayPopups() { return true; }

void OSD::ShowMessageNative(const QString& summary, const QString& message,
                            const QString& icon, const QImage& image) {
#ifdef HAVE_DBUS
  if (!interface_) return;
 
  QVariantMap hints;

  if (!image.isNull()) {
    hints["image_data"] = QVariant(image);
  }

  if (capabilities_.contains("persistence")) {
    hints["resident"] = QVariant(true);
  }

  QStringList actions;
  if (capabilities_.contains("action-icons") && player_) {
    hints["action-icons"] = QVariant(true);
    actions << "media-skip-backward" << tr("Previous");
    actions << "media-skip-forward" << tr("Next");
  }

  // Used by gnome shell
  hints["category"]  = QVariant("x-gnome.music");

  hints["desktop-entry"] = QVariant("clementine");

  QDBusPendingReply<uint> reply =
      interface_->Notify(QCoreApplication::applicationName(), notification_id_, icon, summary,
                         message, actions, hints, timeout_msec_);
  QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(reply, this);
  connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
          SLOT(CallFinished(QDBusPendingCallWatcher*)));
#endif  // HAVE_DBUS
}

#ifdef HAVE_DBUS
void OSD::CallFinished(QDBusPendingCallWatcher* watcher) {
  std::unique_ptr<QDBusPendingCallWatcher> w(watcher);

  QDBusPendingReply<uint> reply = *watcher;
  if (reply.isError()) {
    qLog(Warning) << "Error sending notification" << reply.error().name();
    return;
  }

  uint id = reply.value();
  if (id != 0) {
    notification_id_ = id;
  }
}

void OSD::NotificationClosed(uint id, uint reason) {
  if (notification_id_ == id) {
    notification_id_ = 0;
  }
}

void OSD::NotificationAction(uint id, QString reason) {
  if (reason == "media-skip-backward") {
    player_->Previous();
  } else if (reason == "media-skip-forward") {
    player_->Next();
  }
}

#endif
