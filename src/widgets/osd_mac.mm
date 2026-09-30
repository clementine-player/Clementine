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

#import <Foundation/NSPathUtilities.h>
#import <Foundation/NSURL.h>
#import <UserNotifications/UserNotifications.h>

#include <QDir>
#include <QFile>
#include <QUuid>
#include <QtDebug>

#include "core/logging.h"

namespace {

// Every notification has this identifier, so each one replaces the last
// rather than filling up Notification Center with every song played.
NSString* const kNotificationIdentifier = @"org.clementine-player.now-playing";

// The cover, as a file for the notification to show. Notification Center moves
// the file into its own store once the notification is added.
UNNotificationAttachment* CoverAttachment(const QImage& image) {
  if (image.isNull()) return nil;

  const QString path =
      QDir(QString::fromNSString(NSTemporaryDirectory()))
          .filePath(QString("clementine-cover-%1.png")
                        .arg(QUuid::createUuid().toString(QUuid::WithoutBraces)));
  if (!image.save(path, "PNG")) return nil;

  NSError* error = nil;
  UNNotificationAttachment* attachment =
      [UNNotificationAttachment attachmentWithIdentifier:@"cover"
                                                     URL:[NSURL fileURLWithPath:path.toNSString()]
                                                 options:nil
                                                   error:&error];
  if (!attachment) {
    qLog(Warning) << "Couldn't attach the cover to the notification:"
                  << QString::fromNSString(error.localizedDescription);
    QFile::remove(path);
  }
  return attachment;
}

void Deliver(NSString* title, NSString* body, UNNotificationAttachment* cover) {
  UNUserNotificationCenter* center = [UNUserNotificationCenter currentNotificationCenter];

  // Asking again once the person has answered returns their answer without
  // asking them, so this only prompts before the first notification.
  [center requestAuthorizationWithOptions:UNAuthorizationOptionAlert
                        completionHandler:^(BOOL granted, NSError* error) {
                          if (!granted) {
                            // A build that isn't signed can end up here too.
                            if (error) {
                              qLog(Warning) << "Can't show notifications:"
                                            << QString::fromNSString(
                                                   error.localizedDescription);
                            }
                            return;
                          }

                          UNMutableNotificationContent* content =
                              [[[UNMutableNotificationContent alloc] init] autorelease];
                          content.title = title;
                          content.body = body;
                          if (cover) content.attachments = @[ cover ];

                          UNNotificationRequest* request =
                              [UNNotificationRequest requestWithIdentifier:kNotificationIdentifier
                                                                   content:content
                                                                   trigger:nil];
                          [center addNotificationRequest:request
                                   withCompletionHandler:^(NSError* add_error) {
                                     if (add_error) {
                                       qLog(Warning) << "Couldn't show a notification:"
                                                     << QString::fromNSString(
                                                            add_error.localizedDescription);
                                     }
                                   }];
                        }];
}

}  // namespace

void OSD::Init() {}

bool OSD::SupportsNativeNotifications() { return true; }

bool OSD::SupportsTrayPopups() { return false; }

void OSD::ShowMessageNative(const QString& summary, const QString& message, const QString& icon,
                            const QImage& image) {
  Q_UNUSED(icon);
  Deliver(summary.toNSString(), message.toNSString(), CoverAttachment(image));
}
