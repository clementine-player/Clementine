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

#include <QBuffer>
#include <QByteArray>
#include <QFile>
#include <QtDebug>

#import <GrowlApplicationBridge.h>

#include "core/scoped_nsautorelease_pool.h"
#include "core/scoped_nsobject.h"

@interface GrowlInterface : NSObject<GrowlApplicationBridgeDelegate> {
}
- (void)SendGrowlAlert:(NSString*)message
                 title:(NSString*)title
                 image:(NSData*)image;
- (void)ClickCallback;  // Called when user clicks on notification.
@end

@implementation GrowlInterface

- (id)init {
  if ((self = [super init])) {
    [GrowlApplicationBridge setGrowlDelegate:self];
  }
  return self;
}

- (void)dealloc {
  [super dealloc];
}

- (NSDictionary*)registrationDictionaryForGrowl {
  NSArray* array = [NSArray
      arrayWithObjects:@"next_track", nil];  // Valid notification names.
  NSDictionary* dict = [NSDictionary
      dictionaryWithObjectsAndKeys:[NSNumber numberWithInt:1], @"TicketVersion",
                                   array, @"AllNotifications", array,
                                   @"DefaultNotifications",
                                   @"com.davidsansome.clementine",
                                   @"ApplicationId", nil];
  return dict;
}

- (void)growlNotificationWasClicked:(id)clickContext {
  if (clickContext) {
    [self ClickCallback];
  }
  return;
}

- (void)SendGrowlAlert:(NSString*)message
                 title:(NSString*)title
                 image:(NSData*)image {
  [GrowlApplicationBridge
       notifyWithTitle:title
           description:message
      notificationName:@"next_track"
              iconData:image
              priority:0
              isSticky:NO
          clickContext:@"click_callback"];  // String sent to our callback.
}

- (void)ClickCallback {
  qDebug() << "Growl notification clicked!";
  return;
}

@end

class OSD::GrowlNotificationWrapper {
 public:
  GrowlNotificationWrapper() {
    growl_interface_ = [[GrowlInterface alloc] init];
  }

  ~GrowlNotificationWrapper() { [growl_interface_ release]; }

  void ShowMessage(const QString& summary, const QString& message,
                   const QImage& image) {

    NSString* mac_message =
        [[NSString alloc] initWithUTF8String:message.toUtf8().constData()];
    NSString* mac_summary =
        [[NSString alloc] initWithUTF8String:summary.toUtf8().constData()];

    NSData* image_data = nil;
    // Growl expects raw TIFF data.
    // This is nasty but it keeps the API nice.
    if (!image.isNull()) {
      QByteArray tiff_data;
      QBuffer tiff(&tiff_data);
      image.save(&tiff, "TIFF");
      image_data =
          [NSData dataWithBytes:tiff_data.constData() length:tiff_data.size()];
    }

    [growl_interface_ SendGrowlAlert:mac_message
                               title:mac_summary
                               image:image_data];

    [mac_message release];
    [mac_summary release];
  }

 private:
  GrowlInterface* growl_interface_;
  ScopedNSAutoreleasePool pool_;
};

void OSD::Init() { wrapper_ = new GrowlNotificationWrapper; }

bool OSD::SupportsNativeNotifications() { return true; }

bool OSD::SupportsTrayPopups() { return false; }

namespace {

bool NotificationCenterSupported() {
  return NSClassFromString(@"NSUserNotificationCenter");
}

void SendNotificationCenterMessage(NSString* title, NSString* subtitle) {
  Class clazz = NSClassFromString(@"NSUserNotificationCenter");
  id notification_center = [clazz defaultUserNotificationCenter];

  Class user_notification_class = NSClassFromString(@"NSUserNotification");
  id notification = [[user_notification_class alloc] init];
  [notification setTitle:title];
  [notification setSubtitle:subtitle];

  [notification_center deliverNotification:notification];
}
}

void OSD::ShowMessageNative(const QString& summary, const QString& message,
                            const QString& icon, const QImage& image) {
  Q_UNUSED(icon);

  if (NotificationCenterSupported()) {
    scoped_nsobject<NSString> mac_message(
        [[NSString alloc] initWithUTF8String:message.toUtf8().constData()]);
    scoped_nsobject<NSString> mac_summary(
        [[NSString alloc] initWithUTF8String:summary.toUtf8().constData()]);
    SendNotificationCenterMessage(mac_summary.get(), mac_message.get());
  } else {
    wrapper_->ShowMessage(summary, message, image);
  }
}
