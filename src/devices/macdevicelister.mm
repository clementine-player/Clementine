#include "macdevicelister.h"

#include <CoreFoundation/CFRunLoop.h>
#include <DiskArbitration/DiskArbitration.h>
#include <IOKit/storage/IOBlockStorageDevice.h>
#include <IOKit/storage/IOStorage.h>
#include <IOKit/usb/IOUSBLib.h>

#import <AppKit/NSWorkspace.h>
#import <Foundation/NSNotification.h>
#import <Foundation/NSString.h>

#include <QtDebug>

#include <QString>
#include <QStringList>

@interface NotificationTarget :NSObject {

}

- (void) mediaMounted: (NSNotification*) notification;
- (void) mediaUnmounted: (NSNotification*) notification;

@end

@implementation NotificationTarget

- (id) init {
  qDebug() << Q_FUNC_INFO;
  if ((self = [super init])) {
    [[[NSWorkspace sharedWorkspace] notificationCenter]
        addObserver:self
        selector:@selector(mediaMounted:)
        name:NSWorkspaceDidMountNotification
        object:[NSWorkspace sharedWorkspace]];

    [[[NSWorkspace sharedWorkspace] notificationCenter]
        addObserver:self
        selector:@selector(mediaUnmounted:)
        name:NSWorkspaceDidUnmountNotification
        object:[NSWorkspace sharedWorkspace]];
  }
  return self;
}

- (void) mediaMounted: (NSNotification*) notification {
  NSString* path = [[notification userInfo] objectForKey:@"NSDevicePath"];
  qDebug() << Q_FUNC_INFO << QString::fromUtf8([path UTF8String]);
}

- (void) mediaUnmounted: (NSNotification*) notification {
  NSString* path = [[notification userInfo] objectForKey:@"NSDevicePath"];
  qDebug() << Q_FUNC_INFO << QString::fromUtf8([path UTF8String]);
}

@end


class MacDeviceListerPrivate {
 public:
  MacDeviceListerPrivate() {
    target_ = [[NotificationTarget alloc] init];
  }

 private:
  NotificationTarget* target_;
};


MacDeviceLister::MacDeviceLister() : p_(new MacDeviceListerPrivate) {
}

MacDeviceLister::~MacDeviceLister() {
  qDebug() << Q_FUNC_INFO;
}

void MacDeviceLister::Init() {
  qDebug() << Q_FUNC_INFO;

  notification_port_ = IONotificationPortCreate(kIOMasterPortDefault);
  CFRunLoopSourceRef loop_source =
      IONotificationPortGetRunLoopSource(notification_port_);

  CFRunLoopRef run_loop = CFRunLoopGetCurrent();
  CFRunLoopAddSource(run_loop, loop_source, kCFRunLoopDefaultMode);

  AddNotification(
      kIOFirstMatchNotification,
      kIOUSBDeviceClassName,
      &DeviceAddedCallback);
  AddNotification(
      kIOTerminatedNotification,
      kIOUSBDeviceClassName,
      &DeviceRemovedCallback);

  AddNotification(
      kIOFirstMatchNotification,
      kIOBlockStorageDeviceClass,
      &StorageAddedCallback);


  DASessionRef da_session = DASessionCreate(kCFAllocatorDefault);
  DARegisterDiskAppearedCallback(
      da_session, NULL, &DiskAddedCallback, reinterpret_cast<void*>(this));
  DARegisterDiskDisappearedCallback(
      da_session, NULL, &DiskRemovedCallback, reinterpret_cast<void*>(this));
  DAApprovalSessionRef approval_session = DAApprovalSessionCreate(kCFAllocatorDefault);
  DARegisterDiskMountApprovalCallback(
      approval_session, NULL, &DiskMountCallback, reinterpret_cast<void*>(this));
  DASessionScheduleWithRunLoop(da_session, run_loop, kCFRunLoopDefaultMode);
  DAApprovalSessionScheduleWithRunLoop(approval_session, run_loop, kCFRunLoopDefaultMode);
  CFRelease(da_session);
  CFRelease(approval_session);


  qDebug() << "STARTING LOOP";
  CFRunLoopRun();
  qDebug() << "ENDING LOOP";
}

bool MacDeviceLister::AddNotification(
    const io_name_t type,
    const char* class_name,
    IOServiceMatchingCallback callback) {
  CFMutableDictionaryRef matching_dict = IOServiceMatching(class_name);
  io_iterator_t it;
  kern_return_t ret = IOServiceAddMatchingNotification(
      notification_port_,
      type,
      matching_dict,
      callback,
      reinterpret_cast<void*>(this),
      &it);
  if (ret != KERN_SUCCESS) {
    return false;
  }

  while (IOIteratorNext(it));
  return true;
}

void MacDeviceLister::DeviceAddedCallback(void* refcon, io_iterator_t it) {
  qDebug() << Q_FUNC_INFO;

  io_object_t object;
  while ((object = IOIteratorNext(it))) {
    io_name_t class_name;
    IOObjectGetClass(object, class_name);
    qDebug() << class_name;

    IOObjectRelease(object);
  }
}

void MacDeviceLister::DeviceRemovedCallback(void* refcon, io_iterator_t it) {
  qDebug() << Q_FUNC_INFO;
  while (IOIteratorNext(it));
}

void MacDeviceLister::StorageAddedCallback(void* refcon, io_iterator_t it) {
  qDebug() << Q_FUNC_INFO;

  io_object_t object;
  while ((object = IOIteratorNext(it))) {
    io_name_t class_name;
    IOObjectGetClass(object, class_name);
    qDebug() << class_name;

    io_iterator_t registry_it;
    kern_return_t ret = IORegistryEntryGetParentEntry(
        object, kIOServicePlane, &registry_it);

    io_object_t registry_entry;
    while ((registry_entry = IOIteratorNext(registry_it))) {
      io_name_t name;
      IORegistryEntryGetName(registry_entry, name);
      qDebug() << name;

      IOObjectRelease(registry_entry);
    }


    IOObjectRelease(object);
  }
}

void MacDeviceLister::DiskAddedCallback(DADiskRef disk, void* context) {
  qDebug() << Q_FUNC_INFO;

  io_service_t service = DADiskCopyIOMedia(disk);
  io_iterator_t registry_it;
  IORegistryEntryGetParentEntry(
      service, kIOServicePlane, &registry_it);

  io_object_t registry_entry;
  while ((registry_entry = IOIteratorNext(registry_it))) {
    io_name_t name;
    IORegistryEntryGetName(registry_entry, name);
    qDebug() << name;

    IOObjectRelease(registry_entry);
  }

  qDebug() << DADiskGetBSDName(disk);

  IOObjectRelease(service);
}

void MacDeviceLister::DiskRemovedCallback(DADiskRef, void* context) {
  qDebug() << Q_FUNC_INFO;
}

DADissenterRef MacDeviceLister::DiskMountCallback(DADiskRef disk, void* context) {
  qDebug() << Q_FUNC_INFO << DADiskGetBSDName(disk);
  return NULL;
}

QStringList MacDeviceLister::DeviceUniqueIDs(){return QStringList();}
QStringList MacDeviceLister::DeviceIcons(const QString& id){return QStringList();}
QString MacDeviceLister::DeviceManufacturer(const QString& id){return QString();}
QString MacDeviceLister::DeviceModel(const QString& id){return QString();}
quint64 MacDeviceLister::DeviceCapacity(const QString& id){return 0;}
quint64 MacDeviceLister::DeviceFreeSpace(const QString& id){return 0;}
QVariantMap MacDeviceLister::DeviceHardwareInfo(const QString& id){return QVariantMap();}
QString MacDeviceLister::MakeFriendlyName(const QString& id){return QString();}
QUrl MacDeviceLister::MakeDeviceUrl(const QString& id){return QUrl();}
