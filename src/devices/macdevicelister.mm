#include "macdevicelister.h"

#include <CoreFoundation/CFRunLoop.h>
#include <DiskArbitration/DiskArbitration.h>
#include <IOKit/kext/KextManager.h>
#include <IOKit/IOCFPlugin.h>
#include <IOKit/usb/IOUSBLib.h>

#import <AppKit/NSWorkspace.h>
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSNotification.h>
#import <Foundation/NSPathUtilities.h>
#import <Foundation/NSString.h>
#import <Foundation/NSURL.h>

#include <libmtp.h>

#include <QtDebug>

#include <QString>
#include <QStringList>

#ifndef kUSBSerialNumberString
#define kUSBSerialNumberString "USB Serial Number"
#endif

#ifndef kUSBVendorString
#define kUSBVendorString "USB Vendor Name"
#endif

#ifndef kUSBProductString
#define kUSBProductString "USB Product Name"
#endif

QSet<MacDeviceLister::MTPDevice> MacDeviceLister::sMTPDeviceList;

uint qHash(const MacDeviceLister::MTPDevice& d) {
  return qHash(d.vendor) ^ qHash(d.vendor_id) ^ qHash(d.product) ^ qHash(d.product_id);
}

MacDeviceLister::MacDeviceLister() {
}

MacDeviceLister::~MacDeviceLister() {
  CFRelease(loop_session_);
}

void MacDeviceLister::Init() {
  [[NSAutoreleasePool alloc] init];

  // Populate MTP Device list.
  if (sMTPDeviceList.empty()) {
    LIBMTP_device_entry_t* devices = NULL;
    int num = 0;
    if (LIBMTP_Get_Supported_Devices_List(&devices, &num) != 0) {
      qWarning() << "Failed to get MTP device list";
    } else {
      for (int i = 0; i < num; ++i) {
        LIBMTP_device_entry_t device = devices[i];
        MTPDevice d;
        d.vendor = QString::fromAscii(device.vendor);
        d.vendor_id = device.vendor_id;
        d.product = QString::fromAscii(device.product);
        d.product_id = device.product_id;
      }
    }
  }

  run_loop_ = CFRunLoopGetCurrent();

  // Register for disk mounts/unmounts.
  loop_session_ = DASessionCreate(kCFAllocatorDefault);
  DARegisterDiskAppearedCallback(
      loop_session_, kDADiskDescriptionMatchVolumeMountable, &DiskAddedCallback, reinterpret_cast<void*>(this));
  DARegisterDiskDisappearedCallback(
      loop_session_, NULL, &DiskRemovedCallback, reinterpret_cast<void*>(this));
  DASessionScheduleWithRunLoop(loop_session_, run_loop_, kCFRunLoopDefaultMode);

  // Register for USB device connection/disconnection.
  IONotificationPortRef notification_port = IONotificationPortCreate(kIOMasterPortDefault);
  CFMutableDictionaryRef matching_dict = IOServiceMatching(kIOUSBDeviceClassName);
  io_iterator_t it;
  kern_return_t err = IOServiceAddMatchingNotification(
      notification_port,
      kIOFirstMatchNotification,
      matching_dict,
      &USBDeviceAddedCallback,
      reinterpret_cast<void*>(this),
      &it);
  if (err == KERN_SUCCESS) {
    USBDeviceAddedCallback(this, it);
  } else {
    qWarning() << "Could not add notification on USB device connection";
  }

  CFRunLoopSourceRef io_source = IONotificationPortGetRunLoopSource(notification_port);
  CFRunLoopAddSource(run_loop_, io_source, kCFRunLoopDefaultMode);

  CFRunLoopRun();
}

void MacDeviceLister::ShutDown() {
  CFRunLoopStop(run_loop_);
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

NSObject* GetPropertyForDevice(io_object_t device, CFStringRef key) {
  CFMutableDictionaryRef properties;
  kern_return_t ret = IORegistryEntryCreateCFProperties(
      device, &properties, kCFAllocatorDefault, 0);

  if (ret != KERN_SUCCESS) {
    return nil;
  }

  NSDictionary* dict = (NSDictionary*)properties;
  NSObject* prop = [dict objectForKey:(NSString*)key];
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
  NSDictionary* media_icon = (NSDictionary*)GetPropertyForDevice(device, CFSTR("IOMediaIcon"));
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

void MacDeviceLister::USBDeviceAddedCallback(void* refcon, io_iterator_t it) {
  MacDeviceLister* me = reinterpret_cast<MacDeviceLister*>(refcon);

  io_object_t object;
  while ((object = IOIteratorNext(it))) {
    CFStringRef class_name = IOObjectCopyClass(object);
    if (CFStringCompare(class_name, CFSTR(kIOUSBDeviceClassName), 0) == kCFCompareEqualTo) {
      NSString* vendor = (NSString*)GetPropertyForDevice(object, CFSTR(kUSBVendorString));
      NSString* product = (NSString*)GetPropertyForDevice(object, CFSTR(kUSBProductString));
      NSNumber* vendor_id = (NSNumber*)GetPropertyForDevice(object, CFSTR(kUSBVendorID));
      NSNumber* product_id = (NSNumber*)GetPropertyForDevice(object, CFSTR(kUSBProductID));

      MTPDevice device;
      device.vendor = QString::fromUtf8([vendor UTF8String]);
      device.product = QString::fromUtf8([product UTF8String]);
      device.vendor_id = [vendor_id unsignedShortValue];
      device.product_id = [product_id unsignedShortValue];

      qDebug() << device.vendor
               << device.vendor_id
               << device.product
               << device.product_id;

      // First check the libmtp device list.
      if (sMTPDeviceList.contains(device)) {
        qDebug() << "Matched device to libmtp list!";
        // emit.
        return;
      }

      IOCFPlugInInterface** plugin_interface = NULL;
      SInt32 score;
      kern_return_t err = IOCreatePlugInInterfaceForService(
          object,
          kIOUSBDeviceUserClientTypeID,
          kIOCFPlugInInterfaceID,
          &plugin_interface,
          &score);
      if (err != KERN_SUCCESS) {
        continue;
      }

      IOUSBDeviceInterface** dev = NULL;
      HRESULT result = (*plugin_interface)->QueryInterface(
          plugin_interface,
          CFUUIDGetUUIDBytes(kIOUSBDeviceInterfaceID),
          (LPVOID*)&dev);

      (*plugin_interface)->Release(plugin_interface);

      if (result || !dev) {
        continue;
      }

      err = (*dev)->USBDeviceOpen(dev);
      if (err != kIOReturnSuccess) {
        continue;
      }


      // Request the string descriptor at 0xee.
      // This is a magic string that indicates whether this device supports MTP.

      // Fetch string descriptor length.
      UInt8 desc[256];  // Max descriptor length.
      IOUSBDevRequest req;
      req.bmRequestType = USBmakebmRequestType(kUSBIn, kUSBStandard, kUSBDevice);
      req.bRequest = kUSBRqGetDescriptor;
      req.wValue = (kUSBStringDesc << 8) | 0xee;  // 0xee is the MTP descriptor.
      req.wIndex = 0x0409;  // English
      req.wLength = 2;
      req.pData = &desc;
      err = (*dev)->DeviceRequest(dev, &req);

      if (err != kIOReturnSuccess) {
        continue;
      }

      UInt8 string_len = desc[0];
      if (string_len == 0) {
        continue;
      }

      // Fetch actual string descriptor.
      req.bmRequestType = USBmakebmRequestType(kUSBIn, kUSBStandard, kUSBDevice);
      req.bRequest = kUSBRqGetDescriptor;
      req.wValue = (kUSBStringDesc << 8) | 0xee;
      req.wIndex = 0x0409;
      req.wLength = string_len;
      req.pData = &desc;

      err = (*dev)->DeviceRequest(dev, &req);
      if (err != kIOReturnSuccess) {
        continue;
      }

      // The device actually returned something. That's a good sign.
      // Because this was designed by MS, the characters are in UTF-16 (LE?).
      CFStringRef str = CFStringCreateWithCharacters(NULL, (const UniChar*)(desc + 2), (req.wLenDone-2) / 2);
      char buf[256];
      CFStringGetCString(str, buf, 256, kCFStringEncodingNonLossyASCII);
      CFRelease(str);

      if (QString(buf).startsWith("MSFT100")) {
        // We got the OS descriptor!
        char vendor_code = desc[16];
        req.bmRequestType = USBmakebmRequestType(kUSBIn, kUSBVendor, kUSBDevice);
        req.bRequest = vendor_code;
        req.wValue = 0;
        req.wIndex = 4;
        req.wLength = 256;  // Magic number!
        req.pData = &desc;

        err = (*dev)->DeviceRequest(dev, &req);
        if (err != kIOReturnSuccess) {
          qDebug() << "Getting vendor OS descriptor failed";
          continue;
        }

        // Moar magic!
        if (desc[0] != 0x28) {
          continue;
        }

        if (desc[0x12] != 'M' || desc[0x13] != 'T' || desc[0x14] != 'P') {
          // Not quite.
          continue;
        }

        req.bmRequestType = USBmakebmRequestType(kUSBIn, kUSBVendor, kUSBDevice);
        req.bRequest = vendor_code;
        req.wValue = 0;
        req.wIndex = 5;
        req.wLength = 256;
        req.pData = &desc;

        err = (*dev)->DeviceRequest(dev, &req);
        if (err != kIOReturnSuccess || desc[0] != 0x28) {
          continue;
        }

        if (desc[0x12] != 'M' || desc[0x13] != 'T' || desc[0x14] != 'P') {
          // :-(
          continue;
        }


        // Hurray! We made it!
        qDebug() << "New MTP device detected!";
      }

      (*dev)->USBDeviceClose(dev);
      (*dev)->Release(dev);
    }

    CFRelease(class_name);
    IOObjectRelease(object);
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

QList<QUrl> MacDeviceLister::MakeDeviceUrls(const QString& serial) {
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

  return QList<QUrl>() << ret;
}

QStringList MacDeviceLister::DeviceUniqueIDs() {
  return current_devices_.keys();
}

QVariantList MacDeviceLister::DeviceIcons(const QString& serial) {
  QString bsd_name = current_devices_[serial];
  DASessionRef session = DASessionCreate(kCFAllocatorDefault);
  DADiskRef disk = DADiskCreateFromBSDName(
      kCFAllocatorDefault, session, bsd_name.toAscii().constData());

  io_object_t device = DADiskCopyIOMedia(disk);
  QString icon = GetIconForDevice(device);

  NSDictionary* properties = (NSDictionary*)DADiskCopyDescription(disk);
  NSURL* volume_path = 
      [[properties objectForKey:(NSString*)kDADiskDescriptionVolumePathKey] copy];

  QString path = QString::fromUtf8([[volume_path path] UTF8String]);

  IOObjectRelease(device);
  CFRelease(disk);
  CFRelease(session);

  QVariantList ret;
  ret << GuessIconForPath(path);
  ret << GuessIconForModel(DeviceManufacturer(serial), DeviceModel(serial));
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

  NSNumber* capacity = (NSNumber*)GetPropertyForDevice(device, CFSTR("Size"));

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

QVariantMap MacDeviceLister::DeviceHardwareInfo(const QString& serial){return QVariantMap();}

void MacDeviceLister::UnmountDevice(const QString& serial) {
  QString bsd_name = current_devices_[serial];
  DADiskRef disk = DADiskCreateFromBSDName(
      kCFAllocatorDefault, loop_session_, bsd_name.toAscii().constData());

  DADiskUnmount(disk, kDADiskUnmountOptionDefault, &DiskUnmountCallback, this);

  CFRelease(disk);
}

void MacDeviceLister::DiskUnmountCallback(
    DADiskRef disk, DADissenterRef dissenter, void* context) {
  if (dissenter) {
    qWarning() << "Another app blocked the unmount";
  } else {
    DiskRemovedCallback(disk, context);
  }
}

void MacDeviceLister::UpdateDeviceFreeSpace(const QString& serial) {
  emit DeviceChanged(serial);
}
