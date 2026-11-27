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

#include "macdevicelister.h"
#include "config.h"
#include "core/logging.h"
#include "core/scoped_cftyperef.h"
#include "core/scoped_nsautorelease_pool.h"
#include "core/scoped_nsobject.h"
#include "mtpconnection.h"

#include <CoreFoundation/CFRunLoop.h>
#include <DiskArbitration/DiskArbitration.h>
#include <IOKit/IOCFPlugin.h>
#include <IOKit/kext/KextManager.h>
#include <IOKit/storage/IOCDMedia.h>
#include <IOKit/storage/IOMedia.h>
#include <IOKit/usb/IOUSBLib.h>

#import <AppKit/NSWorkspace.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSNotification.h>
#import <Foundation/NSPathUtilities.h>
#import <Foundation/NSString.h>
#import <Foundation/NSURL.h>

#include <libmtp.h>

#include <QMutex>
#include <QScopeGuard>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QUrlQuery>
#include <QtDebug>

#ifndef kUSBSerialNumberString
#define kUSBSerialNumberString "USB Serial Number"
#endif

#ifndef kUSBVendorString
#define kUSBVendorString "USB Vendor Name"
#endif

#ifndef kUSBProductString
#define kUSBProductString "USB Product Name"
#endif

// io_object_t, io_service_t, io_iterator_t etc. are all typedef'd to unsigned
// int,
// hence the lack of templating here.
class ScopedIOObject {
 public:
  explicit ScopedIOObject(io_object_t object = 0) : object_(object) {}

  ~ScopedIOObject() {
    if (object_) IOObjectRelease(object_);
  }

  io_object_t get() const { return object_; }

 private:
  io_object_t object_;

  Q_DISABLE_COPY(ScopedIOObject);
};

// Helpful MTP & USB links:
// Apple USB device interface guide:
// http://developer.apple.com/mac/library/documentation/DeviceDrivers/Conceptual/USBBook/USBDeviceInterfaces/USBDevInterfaces.html
// Example Apple code for requesting a USB device descriptor:
// http://www.opensource.apple.com/source/IOUSBFamily/IOUSBFamily-208.4.5/USBProber/BusProbeClass.m
// Libmtp's detection code:
// http://libmtp.cvs.sourceforge.net/viewvc/libmtp/libmtp/src/libusb-glue.c?view=markup
// Libusb's Mac code:
// http://www.libusb.org/browser/libusb/libusb/os/darwin_usb.c
// Microsoft OS Descriptors:
// http://www.microsoft.com/whdc/connect/usb/os_desc.mspx
// Symbian docs for implementing the device side:
// http://developer.symbian.org/main/documentation/reference/s3/pdk/GUID-3FF0F248-EDF0-5348-BC43-869CE1B5B415.html
// Libgphoto2 MTP detection code:
// http://www.sfr-fresh.com/unix/privat/libgphoto2-2.4.10.1.tar.gz:a/libgphoto2-2.4.10.1/libgphoto2_port/usb/check-mtp-device.c

QSet<MacDeviceLister::MTPDevice> MacDeviceLister::sMTPDeviceList;

uint qHash(const MacDeviceLister::MTPDevice& d) { return qHash(d.vendor_id) ^ qHash(d.product_id); }

MacDeviceLister::MacDeviceLister() {}

MacDeviceLister::~MacDeviceLister() {
  // Stop CFRunLoopRun() and join thread_ before member destructors and
  // DeviceLister::~DeviceLister() run, as callbacks access this and
  // loop_session_.
  ShutDown();
  if (thread_) {
    thread_->quit();
    thread_->wait();
  }
  if (loop_session_) {
    CFRelease(loop_session_);
    loop_session_ = nullptr;
  }
}

void MacDeviceLister::Init() {
  ScopedNSAutoreleasePool pool;

  // Populate MTP Device list.
  if (sMTPDeviceList.empty()) {
    LIBMTP_device_entry_t* devices = nullptr;
    int num = 0;
    if (LIBMTP_Get_Supported_Devices_List(&devices, &num) != 0) {
      qLog(Warning) << "Failed to get MTP device list";
    } else {
      for (int i = 0; i < num; ++i) {
        LIBMTP_device_entry_t device = devices[i];
        MTPDevice d;
        d.vendor = QString::fromLatin1(device.vendor);
        d.vendor_id = device.vendor_id;
        d.product = QString::fromLatin1(device.product);
        d.product_id = device.product_id;
        d.quirks = device.device_flags;
        sMTPDeviceList << d;
      }
    }

    MTPDevice d;
    d.vendor = "SanDisk";
    d.vendor_id = 0x781;
    d.product = "Sansa Clip+";
    d.product_id = 0x74d0;

    d.quirks = 0x2 | 0x4 | 0x40 | 0x4000;
    sMTPDeviceList << d;
  }

  CFRunLoopRef current_run_loop = CFRunLoopGetCurrent();

  // Register for disk mounts/unmounts.
  loop_session_ = DASessionCreate(kCFAllocatorDefault);
  if (loop_session_) {
    DARegisterDiskAppearedCallback(loop_session_, kDADiskDescriptionMatchVolumeMountable,
                                   &DiskAddedCallback, reinterpret_cast<void*>(this));
    DARegisterDiskDisappearedCallback(loop_session_, nullptr, &DiskRemovedCallback,
                                      reinterpret_cast<void*>(this));
    DASessionScheduleWithRunLoop(loop_session_, current_run_loop, kCFRunLoopDefaultMode);
  } else {
    qLog(Warning) << "Could not create DASession for MacDeviceLister";
  }

  // Register for USB device connection/disconnection.
  IONotificationPortRef notification_port = IONotificationPortCreate(kIOMasterPortDefault);
  CFMutableDictionaryRef matching_dict = IOServiceMatching(kIOUSBDeviceClassName);
  // Keep separate added/removed iterators for the run loop's lifetime; in
  // IOKit, releasing an armed notification iterator disarms notifications.
  io_iterator_t added_it = 0;
  io_iterator_t removed_it = 0;
  CFRunLoopSourceRef io_source = nullptr;

  if (notification_port && matching_dict) {
    // IOServiceAddMatchingNotification consumes one reference each time.
    CFRetain(matching_dict);
    kern_return_t err = IOServiceAddMatchingNotification(
        notification_port, kIOFirstMatchNotification, matching_dict, &USBDeviceAddedCallback,
        reinterpret_cast<void*>(this), &added_it);
    if (err == KERN_SUCCESS) {
      USBDeviceAddedCallback(this, added_it);
    } else {
      qLog(Warning) << "Could not add notification on USB device connection";
    }

    err = IOServiceAddMatchingNotification(notification_port, kIOTerminatedNotification,
                                           matching_dict, &USBDeviceRemovedCallback,
                                           reinterpret_cast<void*>(this), &removed_it);
    if (err == KERN_SUCCESS) {
      USBDeviceRemovedCallback(this, removed_it);
    } else {
      qLog(Warning) << "Could not add notification USB device removal";
    }

    io_source = IONotificationPortGetRunLoopSource(notification_port);
    if (io_source) {
      CFRunLoopAddSource(current_run_loop, io_source, kCFRunLoopDefaultMode);
    }
  } else {
    // Release matching_dict if port creation failed before IOKit consumed it.
    if (matching_dict) CFRelease(matching_dict);
    qLog(Warning) << "Could not create USB notification port/matching dict";
  }

  // Skip CFRunLoopRun() if ShutDown() already ran before Init() reached here,
  // and clear run_loop_ before teardown so ShutDown() never stops a dead loop.
  bool should_run = false;
  {
    QMutexLocker l(&run_loop_mutex_);
    if (!stop_requested_) {
      run_loop_ = current_run_loop;
      should_run = true;
    }
  }

  if (should_run) {
    CFRunLoopRun();
  }

  {
    QMutexLocker l(&run_loop_mutex_);
    run_loop_ = nullptr;
  }

  // Detach DiskArbitration and IOKit sources from this thread's run loop and
  // release notification handles before the thread exits.
  if (loop_session_) {
    DASessionUnscheduleFromRunLoop(loop_session_, current_run_loop, kCFRunLoopDefaultMode);
  }
  if (io_source) {
    CFRunLoopRemoveSource(current_run_loop, io_source, kCFRunLoopDefaultMode);
  }
  if (added_it) IOObjectRelease(added_it);
  if (removed_it) IOObjectRelease(removed_it);
  if (notification_port) IONotificationPortDestroy(notification_port);
}

void MacDeviceLister::ShutDown() {
  QMutexLocker l(&run_loop_mutex_);
  stop_requested_ = true;
  if (run_loop_) {
    CFRunLoopStop(run_loop_);
  }
}

// IOKit helpers.
namespace {

// Caller is responsible for calling CFRelease().
CFTypeRef GetUSBRegistryEntry(io_object_t device, CFStringRef key) {
  if (!device) return nullptr;
  io_iterator_t it = 0;
  if (IORegistryEntryGetParentIterator(device, kIOServicePlane, &it) == KERN_SUCCESS) {
    // Wrap iterators and entries in RAII so early returns never leak.
    ScopedIOObject scoped_it(it);
    io_object_t next = 0;
    while ((next = IOIteratorNext(it))) {
      ScopedIOObject scoped_next(next);
      CFTypeRef registry_entry =
          (CFStringRef)IORegistryEntryCreateCFProperty(next, key, kCFAllocatorDefault, 0);
      if (registry_entry) {
        return registry_entry;
      }

      CFTypeRef ret = GetUSBRegistryEntry(next, key);
      if (ret) {
        return ret;
      }
    }
  }

  return nullptr;
}

QString GetUSBRegistryEntryString(io_object_t device, CFStringRef key) {
  ScopedCFTypeRef<CFStringRef> registry_string((CFStringRef)GetUSBRegistryEntry(device, key));
  if (registry_string) {
    return QString::fromUtf8([(NSString*)registry_string.get() UTF8String]);
  }

  return QString();
}

// Returns a retained (+1) property object that the caller must release.
NSObject* GetPropertyForDevice(io_object_t device, CFStringRef key) {
  if (!device) return nil;
  CFMutableDictionaryRef properties = nullptr;
  kern_return_t ret =
      IORegistryEntryCreateCFProperties(device, &properties, kCFAllocatorDefault, 0);

  if (ret != KERN_SUCCESS) {
    return nil;
  }

  scoped_nsobject<NSDictionary> dict((NSDictionary*)properties);  // Takes ownership.
  NSObject* prop = [dict objectForKey:(NSString*)key];
  if (prop) {
    // The dictionary goes out of scope so we should retain this object.
    [prop retain];
    return prop;
  }

  io_object_t parent = 0;
  ret = IORegistryEntryGetParentEntry(device, kIOServicePlane, &parent);
  if (ret == KERN_SUCCESS) {
    ScopedIOObject scoped_parent(parent);
    return GetPropertyForDevice(parent, key);
  }

  return nil;
}

int GetUSBDeviceClass(io_object_t device) {
  ScopedCFTypeRef<CFTypeRef> interface_class(
      IORegistryEntrySearchCFProperty(device, kIOServicePlane, CFSTR(kUSBInterfaceClass),
                                      kCFAllocatorDefault, kIORegistryIterateRecursively));
  NSNumber* number = (NSNumber*)interface_class.get();
  if (number) {
    int ret = [number unsignedShortValue];
    return ret;
  }
  return 0;
}

QString GetIconForDevice(io_object_t device) {
  scoped_nsobject<NSDictionary> media_icon(
      (NSDictionary*)GetPropertyForDevice(device, CFSTR("IOMediaIcon")));
  if (media_icon) {
    NSString* bundle = (NSString*)[media_icon objectForKey:@"CFBundleIdentifier"];
    NSString* file = (NSString*)[media_icon objectForKey:@"IOBundleResourceFile"];

    scoped_nsobject<NSURL> bundle_url(
        (NSURL*)KextManagerCreateURLForBundleIdentifier(kCFAllocatorDefault, (CFStringRef)bundle));
    if (!bundle_url) return QString();

    QString path = QString::fromUtf8([[bundle_url path] UTF8String]);
    path += "/Contents/Resources/";
    path += QString::fromUtf8([file UTF8String]);
    return path;
  }

  return QString();
}

QString GetSerialForDevice(io_object_t device) {
  QString serial = GetUSBRegistryEntryString(device, CFSTR(kUSBSerialNumberString));
  if (!serial.isEmpty()) {
    return "USB/" + serial;
  }
  return QString();
}

QString GetSerialForMTPDevice(io_object_t device) {
  scoped_nsobject<NSString> serial(
      (NSString*)GetPropertyForDevice(device, CFSTR(kUSBSerialNumberString)));
  return QString(QString("MTP/") + QString::fromUtf8([serial UTF8String]));
}

QString FindDeviceProperty(const QString& bsd_name, CFStringRef property) {
  if (bsd_name.isEmpty()) return QString();
  ScopedCFTypeRef<DASessionRef> session(DASessionCreate(kCFAllocatorDefault));
  if (!session) return QString();
  ScopedCFTypeRef<DADiskRef> disk(
      DADiskCreateFromBSDName(kCFAllocatorDefault, session.get(), bsd_name.toLatin1().constData()));
  if (!disk) return QString();

  ScopedIOObject device(DADiskCopyIOMedia(disk.get()));
  QString ret = GetUSBRegistryEntryString(device.get(), property);
  return ret;
}
}  // namespace

quint64 MacDeviceLister::GetFreeSpace(const QUrl& url) {
  QMutexLocker l(&libmtp_mutex_);
  MtpConnection connection(url);
  if (!connection.is_valid()) {
    qLog(Warning) << "Error connecting to MTP device, couldn't get device free space";
    // Return 0 on failure so -1 does not wrap on unsigned quint64.
    return 0;
  }
  LIBMTP_devicestorage_t* storage = connection.device()->storage;
  quint64 free_bytes = 0;
  while (storage) {
    free_bytes += storage->FreeSpaceInBytes;
    storage = storage->next;
  }
  return free_bytes;
}

quint64 MacDeviceLister::GetCapacity(const QUrl& url) {
  QMutexLocker l(&libmtp_mutex_);
  MtpConnection connection(url);
  if (!connection.is_valid()) {
    qLog(Warning) << "Error connecting to MTP device, couldn't get device capacity";
    // Return 0 on failure so -1 does not wrap on unsigned quint64.
    return 0;
  }
  LIBMTP_devicestorage_t* storage = connection.device()->storage;
  quint64 capacity_bytes = 0;
  while (storage) {
    capacity_bytes += storage->MaxCapacity;
    storage = storage->next;
  }
  return capacity_bytes;
}

void MacDeviceLister::DiskAddedCallback(DADiskRef disk, void* context) {
  MacDeviceLister* me = reinterpret_cast<MacDeviceLister*>(context);

  scoped_nsobject<NSDictionary> properties((NSDictionary*)DADiskCopyDescription(disk));
  if (!properties) return;

#ifdef HAVE_AUDIOCD
  NSString* kind = [properties objectForKey:(NSString*)kDADiskDescriptionMediaKindKey];
  if (kind && strcmp([kind UTF8String], kIOCDMediaClass) == 0) {
    // CD inserted.
    const char* raw_bsd_name = DADiskGetBSDName(disk);
    if (!raw_bsd_name) return;
    QString bsd_name = QString::fromLatin1(raw_bsd_name);
    {
      QMutexLocker l(&me->devices_mutex_);
      me->cd_devices_ << bsd_name;
    }
    emit me->DeviceAdded(bsd_name);
    return;
  }
#endif

  // Adopt the +1 reference returned by -copy.
  scoped_nsobject<NSURL> volume_path(
      [[properties objectForKey:(NSString*)kDADiskDescriptionVolumePathKey] copy]);

  if (volume_path) {
    // Virtual/synthesized disks may have no IOKit media or class name;
    // CFStringCompare() crashes on null CFStringRef.
    ScopedIOObject device(DADiskCopyIOMedia(disk));
    if (!device.get()) return;
    ScopedCFTypeRef<CFStringRef> class_name(IOObjectCopyClass(device.get()));
    if (class_name &&
        CFStringCompare(class_name.get(), CFSTR(kIOMediaClass), 0) == kCFCompareEqualTo) {
      QString vendor = GetUSBRegistryEntryString(device.get(), CFSTR(kUSBVendorString));
      QString product = GetUSBRegistryEntryString(device.get(), CFSTR(kUSBProductString));

      CFMutableDictionaryRef cf_properties = nullptr;
      kern_return_t ret =
          IORegistryEntryCreateCFProperties(device.get(), &cf_properties, kCFAllocatorDefault, 0);

      if (ret == KERN_SUCCESS) {
        scoped_nsobject<NSDictionary> dict((NSDictionary*)cf_properties);  // Takes ownership.
        if ([[dict objectForKey:@"Removable"] intValue] == 1) {
          QString serial = GetSerialForDevice(device.get());
          const char* raw_bsd_name = DADiskGetBSDName(disk);
          if (!serial.isEmpty() && raw_bsd_name) {
            {
              QMutexLocker l(&me->devices_mutex_);
              me->current_devices_[serial] = QString::fromLatin1(raw_bsd_name);
            }
            emit me->DeviceAdded(serial);
          }
        }
      }
    }
  }
}

void MacDeviceLister::DiskRemovedCallback(DADiskRef disk, void* context) {
  MacDeviceLister* me = reinterpret_cast<MacDeviceLister*>(context);
  // We cannot access the USB tree when the disk is removed but we still get
  // the BSD disk name.

  const char* raw_bsd_name = DADiskGetBSDName(disk);
  if (!raw_bsd_name) return;
  QString bsd_name = QString::fromLatin1(raw_bsd_name);
  {
    QMutexLocker l(&me->devices_mutex_);
    // Copy key and unlock devices_mutex_ before emitting DeviceRemoved() so
    // slots querying MacDeviceLister cannot deadlock and iterators stay valid.
    if (me->cd_devices_.remove(bsd_name)) {
      l.unlock();
      emit me->DeviceRemoved(bsd_name);
      return;
    }

    for (QMap<QString, QString>::iterator it = me->current_devices_.begin();
         it != me->current_devices_.end(); ++it) {
      if (it.value() == bsd_name) {
        QString serial = it.key();
        me->current_devices_.erase(it);
        l.unlock();
        emit me->DeviceRemoved(serial);
        break;
      }
    }
  }
}

bool DeviceRequest(IOUSBDeviceInterface** dev, quint8 direction, quint8 type, quint8 recipient,
                   quint8 request_code, quint16 value, quint16 index, quint16 length,
                   QByteArray* data) {
  IOUSBDevRequest req;
  req.bmRequestType = USBmakebmRequestType(direction, type, recipient);
  req.bRequest = request_code;
  req.wValue = value;
  req.wIndex = index;
  req.wLength = length;
  data->resize(256);
  req.pData = data->data();
  kern_return_t err = (*dev)->DeviceRequest(dev, &req);
  if (err != kIOReturnSuccess) {
    return false;
  }
  data->resize(req.wLenDone);
  return true;
}

int GetBusNumber(io_object_t o) {
  io_iterator_t it = 0;
  kern_return_t err = IORegistryEntryGetParentIterator(o, kIOServicePlane, &it);
  if (err != KERN_SUCCESS) {
    return -1;
  }
  // Release iterator, yielded parents, and the +1 NSObject from
  // GetPropertyForDevice() on every loop exit.
  ScopedIOObject scoped_it(it);
  io_object_t parent = 0;
  while ((parent = IOIteratorNext(it))) {
    ScopedIOObject scoped_parent(parent);
    scoped_nsobject<NSObject> bus(GetPropertyForDevice(parent, CFSTR("USBBusNumber")));
    if (bus) {
      NSNumber* bus_num = (NSNumber*)bus.get();
      return [bus_num intValue];
    }
  }

  return -1;
}

void MacDeviceLister::USBDeviceAddedCallback(void* refcon, io_iterator_t it) {
  MacDeviceLister* me = reinterpret_cast<MacDeviceLister*>(refcon);

  io_object_t object = 0;
  while ((object = IOIteratorNext(it))) {
    ScopedCFTypeRef<CFStringRef> class_name(IOObjectCopyClass(object));
    auto release_object = qScopeGuard([object] { IOObjectRelease(object); });

    if (class_name &&
        CFStringCompare(class_name.get(), CFSTR(kIOUSBDeviceClassName), 0) == kCFCompareEqualTo) {
      scoped_nsobject<NSString> vendor(
          (NSString*)GetPropertyForDevice(object, CFSTR(kUSBVendorString)));
      scoped_nsobject<NSString> product(
          (NSString*)GetPropertyForDevice(object, CFSTR(kUSBProductString)));
      scoped_nsobject<NSNumber> vendor_id(
          (NSNumber*)GetPropertyForDevice(object, CFSTR(kUSBVendorID)));
      scoped_nsobject<NSNumber> product_id(
          (NSNumber*)GetPropertyForDevice(object, CFSTR(kUSBProductID)));
      int interface_class = GetUSBDeviceClass(object);
      qLog(Debug) << "Interface class:" << interface_class;

      QString serial = GetSerialForMTPDevice(object);

      MTPDevice device;
      device.vendor = QString::fromUtf8([vendor UTF8String]);
      device.product = QString::fromUtf8([product UTF8String]);
      device.vendor_id = [vendor_id unsignedShortValue];
      device.product_id = [product_id unsignedShortValue];
      device.quirks = 0;

      device.bus = -1;
      device.address = -1;

      // I think we can safely skip Apple products.
      if (device.vendor_id == kAppleVendorID ||
          // Blacklist ilok2 as this probe may be breaking it.
          (device.vendor_id == 0x088e && device.product_id == 0x5036) ||
          // Blacklist eLicenser
          (device.vendor_id == 0x0819 && device.product_id == 0x0101) ||
          // Skip HID devices, printers and hubs.
          interface_class == kUSBHIDInterfaceClass ||
          interface_class == kUSBPrintingInterfaceClass || interface_class == kUSBHubClass) {
        continue;
      }

      scoped_nsobject<NSNumber> addr((NSNumber*)GetPropertyForDevice(object, CFSTR("USB Address")));
      int bus = GetBusNumber(object);
      if (!addr || bus == -1) {
        // Failed to get bus or address number.
        continue;
      }
      device.bus = bus;
      device.address = [addr intValue];

      // First check the libmtp device list.
      QSet<MTPDevice>::const_iterator libmtp_it = sMTPDeviceList.find(device);
      if (libmtp_it != sMTPDeviceList.end()) {
        // Fill in quirks flags from libmtp.
        device.quirks = libmtp_it->quirks;
        me->FoundMTPDevice(device, serial);
        continue;
      }

      IOCFPlugInInterface** plugin_interface = nullptr;
      SInt32 score = 0;
      kern_return_t err = IOCreatePlugInInterfaceForService(
          object, kIOUSBDeviceUserClientTypeID, kIOCFPlugInInterfaceID, &plugin_interface, &score);
      if (err != KERN_SUCCESS || !plugin_interface) {
        continue;
      }

      IOUSBDeviceInterface** dev = nullptr;
      HRESULT result =
          (*plugin_interface)
              ->QueryInterface(plugin_interface, CFUUIDGetUUIDBytes(kIOUSBDeviceInterfaceID),
                               (LPVOID*)&dev);

      (*plugin_interface)->Release(plugin_interface);

      if (result || !dev) {
        continue;
      }

      err = (*dev)->USBDeviceOpen(dev);
      if (err != kIOReturnSuccess) {
        // Release manually since close_dev only closes an opened device.
        (*dev)->Release(dev);
        continue;
      }

      // Automatically close & release usb device at scope exit.
      auto close_dev = qScopeGuard([dev] {
        (*dev)->USBDeviceClose(dev);
        (*dev)->Release(dev);
      });

      // Request the string descriptor at 0xee.
      // This is a magic string that indicates whether this device supports MTP.
      QByteArray data;
      bool ret = DeviceRequest(dev, kUSBIn, kUSBStandard, kUSBDevice, kUSBRqGetDescriptor,
                               (kUSBStringDesc << 8) | 0xee, 0x0409, 2, &data);
      if (!ret || data.isEmpty()) continue;

      // Cast byte 0 unsigned so lengths >= 128 don't sign-extend, and reject
      // descriptors shorter than the 2-byte USB string header.
      UInt8 string_len = static_cast<UInt8>(data[0]);
      if (string_len < 2) continue;

      ret = DeviceRequest(dev, kUSBIn, kUSBStandard, kUSBDevice, kUSBRqGetDescriptor,
                          (kUSBStringDesc << 8) | 0xee, 0x0409, string_len, &data);
      // Require the 2-byte USB header plus at least one UTF-16 code unit.
      if (!ret || data.size() < 4) continue;

      // The device actually returned something. That's a good sign.
      // Because this was designed by MS, the characters are in UTF-16 (LE?).
      QString str = QString::fromUtf16(reinterpret_cast<const char16_t*>(data.constData() + 2),
                                       (data.size() - 2) / 2);

      if (str.startsWith("MSFT100")) {
        // Byte 16 holds bMS_VendorCode; 0x12..0x14 hold the "MTP" ID (0x15).
        if (data.size() < 17) continue;
        // We got the OS descriptor!
        char vendor_code = data[16];
        ret = DeviceRequest(dev, kUSBIn, kUSBVendor, kUSBDevice, vendor_code, 0, 4, 256, &data);
        if (!ret || data.size() < 0x15 || data.at(0) != 0x28) continue;

        if (QString::fromLatin1(data.data() + 0x12, 3) != "MTP") {
          // Not quite.
          continue;
        }

        ret = DeviceRequest(dev, kUSBIn, kUSBVendor, kUSBDevice, vendor_code, 0, 5, 256, &data);
        if (!ret || data.size() < 0x15 || data.at(0) != 0x28) {
          continue;
        }

        if (QString::fromLatin1(data.data() + 0x12, 3) != "MTP") {
          // Not quite.
          continue;
        }
        // Hurray! We made it!
        me->FoundMTPDevice(device, serial);
      }
    }
  }
}

void MacDeviceLister::USBDeviceRemovedCallback(void* refcon, io_iterator_t it) {
  MacDeviceLister* me = reinterpret_cast<MacDeviceLister*>(refcon);
  io_object_t object = 0;
  while ((object = IOIteratorNext(it))) {
    ScopedCFTypeRef<CFStringRef> class_name(IOObjectCopyClass(object));
    auto release_object = qScopeGuard([object] { IOObjectRelease(object); });

    if (class_name &&
        CFStringCompare(class_name.get(), CFSTR(kIOUSBDeviceClassName), 0) == kCFCompareEqualTo) {
      QString serial = GetSerialForMTPDevice(object);
      me->RemovedMTPDevice(serial);
    }
  }
}

void MacDeviceLister::RemovedMTPDevice(const QString& serial) {
  int count = 0;
  {
    QMutexLocker l(&devices_mutex_);
    count = mtp_devices_.remove(serial);
  }
  if (count) {
    qLog(Debug) << "MTP device removed:" << serial;
    emit DeviceRemoved(serial);
  }
}

void MacDeviceLister::FoundMTPDevice(const MTPDevice& device, const QString& serial) {
  qLog(Debug) << "New MTP device detected!" << device.bus << device.address;
  {
    QMutexLocker l(&devices_mutex_);
    mtp_devices_[serial] = device;
  }
  // Unlock devices_mutex_ before MakeDeviceUrls() and libmtp USB I/O, then
  // re-check mtp_devices_.contains() in case it disconnected mid-probe.
  QList<QUrl> urls = MakeDeviceUrls(serial);
  if (!urls.isEmpty()) {
    const quint64 capacity = GetCapacity(urls[0]);
    const quint64 free_space = GetFreeSpace(urls[0]);
    QMutexLocker l(&devices_mutex_);
    if (!mtp_devices_.contains(serial)) return;
    MTPDevice* d = &mtp_devices_[serial];
    d->capacity = capacity;
    d->free_space = free_space;
  }
  emit DeviceAdded(serial);
}

bool IsMTPSerial(const QString& serial) { return serial.startsWith("MTP"); }

bool MacDeviceLister::IsCDDevice(const QString& serial) const {
  QMutexLocker l(&devices_mutex_);
  return cd_devices_.contains(serial);
}

QString MacDeviceLister::MakeFriendlyName(const QString& serial) {
  if (IsMTPSerial(serial)) {
    QMutexLocker l(&devices_mutex_);
    if (!mtp_devices_.contains(serial)) return QString();
    const MTPDevice& device = mtp_devices_[serial];
    if (device.vendor.isEmpty()) {
      return device.product;
    } else {
      return device.vendor + " " + device.product;
    }
  }

  // Snapshot CD vs. disk mapping under one lock so IsCDDevice() and map lookup
  // cannot race; for CDs, cd_devices_ stores the BSD name directly.
  bool is_cd = false;
  QString bsd_name;
  {
    QMutexLocker l(&devices_mutex_);
    is_cd = cd_devices_.contains(serial);
    bsd_name = is_cd ? serial : current_devices_.value(serial);
  }
  if (bsd_name.isEmpty()) return QString();

  ScopedCFTypeRef<DASessionRef> session(DASessionCreate(kCFAllocatorDefault));
  if (!session) return QString();
  ScopedCFTypeRef<DADiskRef> disk(
      DADiskCreateFromBSDName(kCFAllocatorDefault, session.get(), bsd_name.toLatin1().constData()));
  if (!disk) return QString();

  if (is_cd) {
    scoped_nsobject<NSDictionary> properties((NSDictionary*)DADiskCopyDescription(disk.get()));
    NSString* device_name =
        (NSString*)[properties.get() objectForKey:(NSString*)kDADiskDescriptionMediaNameKey];

    return QString::fromUtf8([device_name UTF8String]);
  }

  ScopedIOObject device(DADiskCopyIOMedia(disk.get()));
  if (!device.get()) return QString();

  QString vendor = GetUSBRegistryEntryString(device.get(), CFSTR(kUSBVendorString));
  QString product = GetUSBRegistryEntryString(device.get(), CFSTR(kUSBProductString));

  if (vendor.isEmpty()) {
    return product;
  }
  return vendor + " " + product;
}

QList<QUrl> MacDeviceLister::MakeDeviceUrls(const QString& serial) {
  if (IsMTPSerial(serial)) {
    MTPDevice device;
    {
      QMutexLocker l(&devices_mutex_);
      if (!mtp_devices_.contains(serial)) return QList<QUrl>();
      device = mtp_devices_[serial];
    }
    QString str;
    str = QString::asprintf("gphoto2://usb-%d-%d/", device.bus, device.address);
    QUrlQuery url_query;
    url_query.addQueryItem("vendor", device.vendor);
    url_query.addQueryItem("vendor_id", QString::number(device.vendor_id));
    url_query.addQueryItem("product", device.product);
    url_query.addQueryItem("product_id", QString::number(device.product_id));
    url_query.addQueryItem("quirks", QString::number(device.quirks));
    QUrl url(str);
    url.setQuery(url_query);
    return QList<QUrl>() << url;
  }

  if (IsCDDevice(serial)) {
    return QList<QUrl>() << QUrl(QString("cdda:///dev/r" + serial));
  }

  QString bsd_name;
  {
    QMutexLocker l(&devices_mutex_);
    if (!current_devices_.contains(serial)) return QList<QUrl>();
    bsd_name = current_devices_[serial];
  }
  ScopedCFTypeRef<DASessionRef> session(DASessionCreate(kCFAllocatorDefault));
  if (!session) return QList<QUrl>();
  ScopedCFTypeRef<DADiskRef> disk(
      DADiskCreateFromBSDName(kCFAllocatorDefault, session.get(), bsd_name.toLatin1().constData()));
  if (!disk) return QList<QUrl>();

  scoped_nsobject<NSDictionary> properties((NSDictionary*)DADiskCopyDescription(disk.get()));
  if (!properties) return QList<QUrl>();
  scoped_nsobject<NSURL> volume_path(
      [[properties objectForKey:(NSString*)kDADiskDescriptionVolumePathKey] copy]);
  if (!volume_path) return QList<QUrl>();

  QString path = QString::fromUtf8([[volume_path path] UTF8String]);
  QUrl ret = MakeUrlFromLocalPath(path);

  return QList<QUrl>() << ret;
}

QStringList MacDeviceLister::DeviceUniqueIDs() {
  QMutexLocker l(&devices_mutex_);
  return current_devices_.keys() + mtp_devices_.keys();
}

QVariantList MacDeviceLister::DeviceIcons(const QString& serial) {
  if (IsMTPSerial(serial)) {
    return QVariantList();
  }

  if (IsCDDevice(serial)) {
    return QVariantList() << "media-optical";
  }

  QString bsd_name;
  {
    QMutexLocker l(&devices_mutex_);
    if (!current_devices_.contains(serial)) return QVariantList();
    bsd_name = current_devices_[serial];
  }
  ScopedCFTypeRef<DASessionRef> session(DASessionCreate(kCFAllocatorDefault));
  if (!session) return QVariantList();
  ScopedCFTypeRef<DADiskRef> disk(
      DADiskCreateFromBSDName(kCFAllocatorDefault, session.get(), bsd_name.toLatin1().constData()));
  if (!disk) return QVariantList();

  ScopedIOObject device(DADiskCopyIOMedia(disk.get()));
  QString icon = GetIconForDevice(device.get());

  scoped_nsobject<NSDictionary> properties((NSDictionary*)DADiskCopyDescription(disk.get()));
  scoped_nsobject<NSURL> volume_path(
      [[properties objectForKey:(NSString*)kDADiskDescriptionVolumePathKey] copy]);

  QString path = QString::fromUtf8([[volume_path path] UTF8String]);

  QVariantList ret;
  ret << GuessIconForPath(path);
  ret << GuessIconForModel(DeviceManufacturer(serial), DeviceModel(serial));
  if (!icon.isEmpty()) {
    ret << icon;
  }
  return ret;
}

QString MacDeviceLister::DeviceManufacturer(const QString& serial) {
  if (IsMTPSerial(serial)) {
    QMutexLocker l(&devices_mutex_);
    return mtp_devices_.value(serial).vendor;
  }
  QString bsd_name;
  {
    QMutexLocker l(&devices_mutex_);
    bsd_name = current_devices_.value(serial);
  }
  return FindDeviceProperty(bsd_name, CFSTR(kUSBVendorString));
}

QString MacDeviceLister::DeviceModel(const QString& serial) {
  if (IsMTPSerial(serial)) {
    QMutexLocker l(&devices_mutex_);
    return mtp_devices_.value(serial).product;
  }
  QString bsd_name;
  {
    QMutexLocker l(&devices_mutex_);
    bsd_name = current_devices_.value(serial);
  }
  return FindDeviceProperty(bsd_name, CFSTR(kUSBProductString));
}

quint64 MacDeviceLister::DeviceCapacity(const QString& serial) {
  if (IsMTPSerial(serial)) {
    QMutexLocker l(&devices_mutex_);
    return mtp_devices_.value(serial).capacity;
  }
  QString bsd_name;
  {
    QMutexLocker l(&devices_mutex_);
    if (!current_devices_.contains(serial)) return 0;
    bsd_name = current_devices_[serial];
  }
  ScopedCFTypeRef<DASessionRef> session(DASessionCreate(kCFAllocatorDefault));
  if (!session) return 0;
  ScopedCFTypeRef<DADiskRef> disk(
      DADiskCreateFromBSDName(kCFAllocatorDefault, session.get(), bsd_name.toLatin1().constData()));
  if (!disk) return 0;

  ScopedIOObject device(DADiskCopyIOMedia(disk.get()));
  if (!device.get()) return 0;

  scoped_nsobject<NSNumber> capacity((NSNumber*)GetPropertyForDevice(device.get(), CFSTR("Size")));
  return capacity ? [capacity unsignedLongLongValue] : 0;
}

quint64 MacDeviceLister::DeviceFreeSpace(const QString& serial) {
  if (IsMTPSerial(serial)) {
    QMutexLocker l(&devices_mutex_);
    return mtp_devices_.value(serial).free_space;
  }
  QString bsd_name;
  {
    QMutexLocker l(&devices_mutex_);
    if (!current_devices_.contains(serial)) return 0;
    bsd_name = current_devices_[serial];
  }
  ScopedCFTypeRef<DASessionRef> session(DASessionCreate(kCFAllocatorDefault));
  if (!session) return 0;
  ScopedCFTypeRef<DADiskRef> disk(
      DADiskCreateFromBSDName(kCFAllocatorDefault, session.get(), bsd_name.toLatin1().constData()));
  if (!disk) return 0;

  scoped_nsobject<NSDictionary> properties((NSDictionary*)DADiskCopyDescription(disk.get()));
  if (!properties) return 0;
  scoped_nsobject<NSURL> volume_path(
      [[properties objectForKey:(NSString*)kDADiskDescriptionVolumePathKey] copy]);
  if (!volume_path) return 0;

  NSNumber* value = nil;
  NSError* error = nil;
  if ([volume_path getResourceValue:&value forKey:NSURLVolumeAvailableCapacityKey error:&error] &&
      value) {
    return [value unsignedLongLongValue];
  }
  return 0;
}

QVariantMap MacDeviceLister::DeviceHardwareInfo(const QString& serial) { return QVariantMap(); }

bool MacDeviceLister::AskForScan(const QString& serial) const { return !IsCDDevice(serial); }

void MacDeviceLister::UnmountDevice(const QString& serial) {
  if (IsMTPSerial(serial)) return;

  QString bsd_name;
  {
    QMutexLocker l(&devices_mutex_);
    if (!current_devices_.contains(serial)) return;
    bsd_name = current_devices_[serial];
  }
  // DADiskUnmount() dispatches DiskUnmountCallback on loop_session_'s run
  // loop; loop_session_ may be null if Init() failed or hasn't run yet.
  if (!loop_session_ || bsd_name.isEmpty()) return;
  ScopedCFTypeRef<DADiskRef> disk(
      DADiskCreateFromBSDName(kCFAllocatorDefault, loop_session_, bsd_name.toLatin1().constData()));
  if (!disk) return;

  DADiskUnmount(disk.get(), kDADiskUnmountOptionDefault, &DiskUnmountCallback, this);
}

void MacDeviceLister::DiskUnmountCallback(DADiskRef disk, DADissenterRef dissenter, void* context) {
  if (dissenter) {
    qLog(Warning) << "Another app blocked the unmount";
  } else {
    DiskRemovedCallback(disk, context);
  }
}

void MacDeviceLister::UpdateDeviceFreeSpace(const QString& serial) {
  if (IsMTPSerial(serial)) {
    bool exists = false;
    {
      QMutexLocker l(&devices_mutex_);
      exists = mtp_devices_.contains(serial);
    }
    if (exists) {
      // Release devices_mutex_ across libmtp USB I/O and re-verify presence.
      QList<QUrl> urls = MakeDeviceUrls(serial);
      if (!urls.isEmpty()) {
        const quint64 free_space = GetFreeSpace(urls[0]);
        QMutexLocker l(&devices_mutex_);
        if (mtp_devices_.contains(serial)) {
          mtp_devices_[serial].free_space = free_space;
        }
      }
    }
  }
  emit DeviceChanged(serial);
}
