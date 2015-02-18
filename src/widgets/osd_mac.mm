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

#import <Foundation/NSUserNotification.h>

#include <QBuffer>
#include <QByteArray>
#include <QFile>
#include <QtDebug>

#include "core/scoped_nsobject.h"

namespace {

void SendNotificationCenterMessage(NSString* title, NSString* subtitle) {
  NSUserNotificationCenter* notification_center =
      [NSUserNotificationCenter defaultUserNotificationCenter];

  NSUserNotification* notification = [[NSUserNotification alloc] init];
  [notification setTitle:title];
  [notification setSubtitle:subtitle];

  [notification_center deliverNotification:notification];
}

}  // namespace

void OSD::Init() {}

bool OSD::SupportsNativeNotifications() { return true; }

bool OSD::SupportsTrayPopups() { return false; }

void OSD::ShowMessageNative(const QString& summary, const QString& message,
                            const QString& icon, const QImage& image) {
  Q_UNUSED(icon);
  scoped_nsobject<NSString> mac_message(
      [[NSString alloc] initWithUTF8String:message.toUtf8().constData()]);
  scoped_nsobject<NSString> mac_summary(
      [[NSString alloc] initWithUTF8String:summary.toUtf8().constData()]);
  SendNotificationCenterMessage(mac_summary.get(), mac_message.get());
}
