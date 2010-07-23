#ifndef MACDEVICELISTER_H
#define MACDEVICELISTER_H

#include "devicelister.h"

#include <QThread>

#include <DiskArbitration/DADisk.h>
#include <DiskArbitration/DADissenter.h>
#include <IOKit/IOKitLib.h>

#include <boost/scoped_ptr.hpp>

class MacDeviceListerPrivate;

class MacDeviceLister : public DeviceLister {
  Q_OBJECT
 public:
  MacDeviceLister();
  ~MacDeviceLister();

  virtual QStringList DeviceUniqueIDs();
  virtual QStringList DeviceIcons(const QString& id);
  virtual QString DeviceManufacturer(const QString& id);
  virtual QString DeviceModel(const QString& id);
  virtual quint64 DeviceCapacity(const QString& id);
  virtual quint64 DeviceFreeSpace(const QString& id);
  virtual QVariantMap DeviceHardwareInfo(const QString& id);
  virtual QString MakeFriendlyName(const QString& id);
  virtual QUrl MakeDeviceUrl(const QString& id);

 private:
  virtual void Init();

  bool AddNotification(
      const io_name_t type,
      const char* class_name,
      IOServiceMatchingCallback callback);

  static void DeviceAddedCallback(void* refcon, io_iterator_t it);
  static void DeviceRemovedCallback(void* refcon, io_iterator_t it);

  static void StorageAddedCallback(void* refcon, io_iterator_t it);

  static void DiskAddedCallback(DADiskRef disk, void* context);
  static void DiskRemovedCallback(DADiskRef disk, void* context);

  static DADissenterRef DiskMountCallback(DADiskRef disk, void* context);

  IONotificationPortRef notification_port_;

  boost::scoped_ptr<MacDeviceListerPrivate> p_;
};

#endif
