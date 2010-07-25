#include "macdevicelister.h"

#include <CoreFoundation/CFRunLoop.h>
#include <DiskArbitration/DiskArbitration.h>
#include <IOKit/kext/KextManager.h>
#include <IOKit/usb/IOUSBLib.h>

#import <AppKit/NSWorkspace.h>
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSNotification.h>
#import <Foundation/NSPathUtilities.h>
#import <Foundation/NSString.h>
#import <Foundation/NSURL.h>

#include <QtDebug>

#include <QString>
#include <QStringList>

MacDeviceLister::MacDeviceLister() {
}

MacDeviceLister::~MacDeviceLister() {
  CFRelease(loop_session_);
}

void MacDeviceLister::Init() {
  [[NSAutoreleasePool alloc] init];

  CFRunLoopRef run_loop = CFRunLoopGetCurrent();

  loop_session_ = DASessionCreate(kCFAllocatorDefault);
  DARegisterDiskAppearedCallback(
      loop_session_, kDADiskDescriptionMatchVolumeMountable, &DiskAddedCallback, reinterpret_cast<void*>(this));
  DARegisterDiskDisappearedCallback(
      loop_session_, NULL, &DiskRemovedCallback, reinterpret_cast<void*>(this));
  DASessionScheduleWithRunLoop(loop_session_, run_loop, kCFRunLoopDefaultMode);

  CFRunLoopRun();
}


// IOKit helpers.
namespace {

CFTypeRef GetUSBRegistryEntry(io_object_t device, CFStringRef key) {
  io_iterator_t it;
  if (IORegistryEntryGetParentIterator(device, kIOServicePlane, &it) == KERN_SUCCESS) {
    io_object_t next;
    while ((next = IOIteratorNext(it))) {
      CFTypeRef registry_entry = (CFStringRef)IORegistryEntryCreateCFProperty(
          next, key, kCFAllocatorDefault, 0);
      if (registry_entry) {
        IOObjectRelease(next);
        IOObjectRelease(it);
        return registry_entry;
      }

      CFTypeRef ret = GetUSBRegistryEntry(next, key);
      if (ret) {
        IOObjectRelease(next);
        IOObjectRelease(it);
        return ret;
      }

      IOObjectRelease(next);
    }
  }

  IOObjectRelease(it);
  return NULL;
}

QString GetUSBRegistryEntryString(io_object_t device, CFStringRef key) {
  CFStringRef registry_string = (CFStringRef)GetUSBRegistryEntry(device, key);
  if (registry_string) {
    QString ret = QString::fromUtf8([(NSString*)registry_string UTF8String]);
    CFRelease(registry_string);
    return ret;
  }

  return NULL;
}

quint64 GetUSBRegistryEntryInt64(io_object_t device, CFStringRef key) {
  CFNumberRef registry_num = (CFNumberRef)GetUSBRegistryEntry(device, key);
  if (registry_num) {
    qint64 ret = -1;
    Boolean result = CFNumberGetValue(registry_num, kCFNumberLongLongType, &ret);
    if (!result || ret < 0) {
      return 0;
    } else {
      return ret;
    }
  }

  return 0;
}

NSObject* GetPropertyForDevice(io_object_t device, NSString* key) {
  CFMutableDictionaryRef properties;
  kern_return_t ret = IORegistryEntryCreateCFProperties(
      device, &properties, kCFAllocatorDefault, 0);

  if (ret != KERN_SUCCESS) {
    return nil;
  }

  NSDictionary* dict = (NSDictionary*)properties;
  NSObject* prop = [dict objectForKey:key];
  if (prop) {
    return prop;
  }

  io_object_t parent;
  ret = IORegistryEntryGetParentEntry(device, kIOServicePlane, &parent);
  if (ret == KERN_SUCCESS) {
    return GetPropertyForDevice(parent, key);
  }

  return nil;
}

QString GetIconForDevice(io_object_t device) {
  NSDictionary* media_icon = (NSDictionary*)GetPropertyForDevice(device, @"IOMediaIcon");
  if (media_icon) {
    NSString* bundle = (NSString*)[media_icon objectForKey:@"CFBundleIdentifier"];
    NSString* file = (NSString*)[media_icon objectForKey:@"IOBundleResourceFile"];

    NSURL* bundle_url = (NSURL*)KextManagerCreateURLForBundleIdentifier(
        kCFAllocatorDefault, (CFStringRef)bundle);

    QString path = QString::fromUtf8([[bundle_url path] UTF8String]);
    path += "/Contents/Resources/";
    path += [file UTF8String];
    return path;
  }

  return QString();
}

QString GetSerialForDevice(io_object_t device) {
  return QString(
      "USB/" + GetUSBRegistryEntryString(device, CFSTR(kUSBSerialNumberString)));
}

QString FindDeviceProperty(const QString& bsd_name, CFStringRef property) {
  DASessionRef session = DASessionCreate(kCFAllocatorDefault);
  DADiskRef disk = DADiskCreateFromBSDName(
      kCFAllocatorDefault, session, bsd_name.toAscii().constData());

  io_object_t device = DADiskCopyIOMedia(disk);
  QString ret = GetUSBRegistryEntryString(device, property);
  IOObjectRelease(device);

  CFRelease(disk);
  CFRelease(session);

  return ret;
}

}

void MacDeviceLister::DiskAddedCallback(DADiskRef disk, void* context) {
  MacDeviceLister* me = reinterpret_cast<MacDeviceLister*>(context);

  NSDictionary* properties = (NSDictionary*)DADiskCopyDescription(disk);
  NSURL* volume_path = 
      [[properties objectForKey:(NSString*)kDADiskDescriptionVolumePathKey] copy];

  if (volume_path) {
    io_object_t device = DADiskCopyIOMedia(disk);
    QString vendor = GetUSBRegistryEntryString(device, CFSTR(kUSBVendorString));
    QString product = GetUSBRegistryEntryString(device, CFSTR(kUSBProductString));

    CFMutableDictionaryRef properties;
    kern_return_t ret = IORegistryEntryCreateCFProperties(
        device, &properties, kCFAllocatorDefault, 0);

    if (ret == KERN_SUCCESS) {
      NSDictionary* dict = (NSDictionary*)properties;
      if ([[dict objectForKey:@"Removable"] intValue] == 1) {
        QString serial = GetSerialForDevice(device);
        me->current_devices_[serial] = QString(DADiskGetBSDName(disk));
        emit me->DeviceAdded(serial);
      }
    }

    IOObjectRelease(device);
  }
}

void MacDeviceLister::DiskRemovedCallback(DADiskRef disk, void* context) {
  MacDeviceLister* me = reinterpret_cast<MacDeviceLister*>(context);
  // We cannot access the USB tree when the disk is removed but we still get
  // the BSD disk name.
  for (QMap<QString, QString>::iterator it = me->current_devices_.begin();
       it != me->current_devices_.end(); ++it) {
    if (it.value() == QString::fromLocal8Bit(DADiskGetBSDName(disk))) {
      emit me->DeviceRemoved(it.key());
      me->current_devices_.erase(it);
      break;
    }
  }
}

QString MacDeviceLister::MakeFriendlyName(const QString& serial) {
  QString bsd_name = current_devices_[serial];
  DASessionRef session = DASessionCreate(kCFAllocatorDefault);
  DADiskRef disk = DADiskCreateFromBSDName(
      kCFAllocatorDefault, session, bsd_name.toAscii().constData());

  io_object_t device = DADiskCopyIOMedia(disk);
  QString vendor = GetUSBRegistryEntryString(device, CFSTR(kUSBVendorString));
  QString product = GetUSBRegistryEntryString(device, CFSTR(kUSBProductString));
  IOObjectRelease(device);

  CFRelease(disk);
  CFRelease(session);

  if (vendor.isEmpty()) {
    return product;
  }
  return vendor + " " + product;
}

QUrl MacDeviceLister::MakeDeviceUrl(const QString& serial) {
  QString bsd_name = current_devices_[serial];
  DASessionRef session = DASessionCreate(kCFAllocatorDefault);
  DADiskRef disk = DADiskCreateFromBSDName(
      kCFAllocatorDefault, session, bsd_name.toAscii().constData());

  NSDictionary* properties = (NSDictionary*)DADiskCopyDescription(disk);
  NSURL* volume_path = 
      [[properties objectForKey:(NSString*)kDADiskDescriptionVolumePathKey] copy];

  QString path = [[volume_path path] UTF8String];
  QUrl ret = MakeUrlFromLocalPath(path);

  CFRelease(disk);
  CFRelease(session);

  return ret;
}

QStringList MacDeviceLister::DeviceUniqueIDs() {
  return current_devices_.keys();
}

QStringList MacDeviceLister::DeviceIcons(const QString& serial) {
  QString bsd_name = current_devices_[serial];
  DASessionRef session = DASessionCreate(kCFAllocatorDefault);
  DADiskRef disk = DADiskCreateFromBSDName(
      kCFAllocatorDefault, session, bsd_name.toAscii().constData());

  io_object_t device = DADiskCopyIOMedia(disk);
  QString icon = GetIconForDevice(device);

  IOObjectRelease(device);
  CFRelease(disk);
  CFRelease(session);

  QStringList ret;
  if (!icon.isEmpty()) {
    ret << icon;
  }
  return ret;
}

QString MacDeviceLister::DeviceManufacturer(const QString& serial){
  return FindDeviceProperty(current_devices_[serial], CFSTR(kUSBVendorString));
}

QString MacDeviceLister::DeviceModel(const QString& serial){
  return FindDeviceProperty(current_devices_[serial], CFSTR(kUSBProductString));
}

quint64 MacDeviceLister::DeviceCapacity(const QString& serial){
  QString bsd_name = current_devices_[serial];
  DASessionRef session = DASessionCreate(kCFAllocatorDefault);
  DADiskRef disk = DADiskCreateFromBSDName(
      kCFAllocatorDefault, session, bsd_name.toAscii().constData());

  io_object_t device = DADiskCopyIOMedia(disk);

  NSNumber* capacity = (NSNumber*)GetPropertyForDevice(device, @"Size");

  quint64 ret = [capacity unsignedLongLongValue];

  IOObjectRelease(device);

  CFRelease(disk);
  CFRelease(session);

  return ret;
}

quint64 MacDeviceLister::DeviceFreeSpace(const QString& serial){
  QString bsd_name = current_devices_[serial];
  DASessionRef session = DASessionCreate(kCFAllocatorDefault);
  DADiskRef disk = DADiskCreateFromBSDName(
      kCFAllocatorDefault, session, bsd_name.toAscii().constData());

  NSDictionary* properties = (NSDictionary*)DADiskCopyDescription(disk);
  NSURL* volume_path = 
      [[properties objectForKey:(NSString*)kDADiskDescriptionVolumePathKey] copy];

  FSRef ref;
  OSStatus err = FSPathMakeRef(
      (const UInt8*)[[volume_path path] fileSystemRepresentation], &ref, NULL);
  if (err == noErr) {
    FSCatalogInfo catalog;
    err = FSGetCatalogInfo(&ref, kFSCatInfoVolume, &catalog, NULL, NULL, NULL);
    if (err == noErr) {
      FSVolumeRefNum volume_ref = catalog.volume;
      FSVolumeInfo info;
      err = FSGetVolumeInfo(
          volume_ref, 0, NULL, kFSVolInfoSizes, &info, NULL, NULL);
      UInt64 free_bytes = info.freeBytes;
      return free_bytes;
    }
  }

  return 0;
}

QVariantMap MacDeviceLister::DeviceHardwareInfo(const QString& id){return QVariantMap();}

void MacDeviceLister::UnmountDevice(const QString& serial) {
  QString bsd_name = current_devices_[serial];
  DADiskRef disk = DADiskCreateFromBSDName(
      kCFAllocatorDefault, loop_session_, bsd_name.toAscii().constData());

  DADiskUnmount(disk, kDADiskUnmountOptionDefault, &DiskUnmountCallback, this);

  CFRelease(disk);
}

void MacDeviceLister::DiskUnmountCallback(
    DADiskRef disk, DADissenterRef dissenter, void* context) {
  MacDeviceLister* me = reinterpret_cast<MacDeviceLister*>(context);
  if (dissenter) {
    qDebug() << "Another app blocked the unmount";
  } else {
    DiskRemovedCallback(disk, context);
  }
}
