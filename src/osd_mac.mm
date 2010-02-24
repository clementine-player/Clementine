#include "osd.h"

#include <QBuffer>
#include <QByteArray>
#include <QFile>
#include <QtDebug>

#import <GrowlApplicationBridge.h>

@interface GrowlInterface :NSObject <GrowlApplicationBridgeDelegate> {
}
-(void) SendGrowlAlert:(NSString*)message title:(NSString*)title image:(NSData*)image;
-(void) ClickCallback;  // Called when user clicks on notification.
@end


@implementation GrowlInterface

-(id) init {
  if ((self = [super init])) {
    [GrowlApplicationBridge setGrowlDelegate:self];
  }
  return self;
}

-(void) dealloc {
  [super dealloc];
}

-(NSDictionary*) registrationDictionaryForGrowl {
  NSArray* array = [NSArray arrayWithObjects:@"next_track", nil];  // Valid notification names.
  NSDictionary* dict = [NSDictionary dictionaryWithObjectsAndKeys:
      [NSNumber numberWithInt:1],
      @"TicketVersion",
      array,
      @"AllNotifications",
      array,
      @"DefaultNotifications",
      nil];
  return dict;
}

-(void) growlNotificationWasClicked:(id)clickContext {
  if (clickContext) {
    [self ClickCallback];
  }
  return;
}

-(void) SendGrowlAlert:(NSString*)message title:(NSString*)title image:(NSData*)image {
  [GrowlApplicationBridge notifyWithTitle:title
                          description:message
                          notificationName:@"next_track"
                          iconData:image
                          priority:0
                          isSticky:NO
                          clickContext:@"click_callback"];  // String sent to our callback.
}

-(void) ClickCallback {
  qDebug() << "Growl notification clicked!";
  return;
}

@end

class OSD::GrowlNotificationWrapper {
 public:
  GrowlNotificationWrapper() {
    pool_ = [[NSAutoreleasePool alloc] init];
    growl_interface_ = [[GrowlInterface alloc] init];
  }

  ~GrowlNotificationWrapper() {
    [growl_interface_ release];
    [pool_ release];
  }

  void ShowMessage(const QString& summary,
                   const QString& message,
                   const QImage& image) {

    NSString* mac_message = [[NSString alloc] initWithUTF8String:message.toUtf8().data()];
    NSString* mac_summary = [[NSString alloc] initWithUTF8String:summary.toUtf8().data()];

    NSData* image_data = nil;
    // Growl expects raw TIFF data.
    // This is nasty but it keeps the API nice.
    if (!image.isNull()) {
      QByteArray tiff_data;
      QBuffer tiff(&tiff_data);
      image.save(&tiff, "TIFF");
      image_data = [NSData dataWithBytes:tiff_data.constData() length:tiff_data.size()];
    }

    [growl_interface_ SendGrowlAlert:mac_message
                      title:mac_summary
                      image:image_data];

    [image_data release];
    [mac_message release];
    [mac_summary release];
  }

 private:
  GrowlInterface* growl_interface_;
  NSAutoreleasePool* pool_;
};

void OSD::Init() {
  wrapper_ = new GrowlNotificationWrapper;
}

bool OSD::CanShowNativeMessages() const {
  return true;
}

void OSD::ShowMessageNative(const QString& summary, const QString& message,
                            const QString& icon) {
  Q_UNUSED(icon);
  wrapper_->ShowMessage(summary, message, QImage());
}

void OSD::ShowMessageNative(const QString& summary,
                            const QString& message,
                            const QImage& image) {
  wrapper_->ShowMessage(summary, message, image);
}


