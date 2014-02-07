#ifndef MACDEVICELISTER_H
#define MACDEVICELISTER_H

#include "devicelister.h"

#include <QMutex>
#include <QSet>
#include <QThread>

#include <DiskArbitration/DADisk.h>
#include <DiskArbitration/DADissenter.h>
#include <IOKit/IOKitLib.h>

class MacDeviceLister : public DeviceLister {
  Q_OBJECT
 public:
  MacDeviceLister();
  ~MacDeviceLister();

  virtual QStringList DeviceUniqueIDs();
  virtual QVariantList DeviceIcons(const QString& id);
  virtual QString DeviceManufacturer(const QString& id);
  virtual QString DeviceModel(const QString& id);
  virtual quint64 DeviceCapacity(const QString& id);
  virtual quint64 DeviceFreeSpace(const QString& id);
  virtual QVariantMap DeviceHardwareInfo(const QString& id);
  virtual bool AskForScan(const QString& serial) const;
  virtual QString MakeFriendlyName(const QString& id);
  virtual QList<QUrl> MakeDeviceUrls(const QString& id);

  virtual void UnmountDevice(const QString& id);
  virtual void UpdateDeviceFreeSpace(const QString& id);

  struct MTPDevice {
    MTPDevice() : capacity(0), free_space(0) {}
    QString vendor;
    QString product;
    quint16 vendor_id;
    quint16 product_id;

    int quirks;
    int bus;
    int address;

    quint64 capacity;
    quint64 free_space;
  };

 public slots:
  virtual void ShutDown();

 private:
  virtual void Init();

  static void DiskAddedCallback(DADiskRef disk, void* context);
  static void DiskRemovedCallback(DADiskRef disk, void* context);
  static void USBDeviceAddedCallback(void* refcon, io_iterator_t it);
  static void USBDeviceRemovedCallback(void* refcon, io_iterator_t it);

  static void DiskUnmountCallback(DADiskRef disk, DADissenterRef dissenter,
                                  void* context);

  void FoundMTPDevice(const MTPDevice& device, const QString& serial);
  void RemovedMTPDevice(const QString& serial);

  quint64 GetFreeSpace(const QUrl& url);
  quint64 GetCapacity(const QUrl& url);

  bool IsCDDevice(const QString& serial) const;

  DASessionRef loop_session_;
  CFRunLoopRef run_loop_;

  QMap<QString, QString> current_devices_;
  QMap<QString, MTPDevice> mtp_devices_;
  QSet<QString> cd_devices_;

  QMutex libmtp_mutex_;

  static QSet<MTPDevice> sMTPDeviceList;
};

uint qHash(const MacDeviceLister::MTPDevice& device);
inline bool operator==(const MacDeviceLister::MTPDevice& a,
                       const MacDeviceLister::MTPDevice& b) {
  return (a.vendor_id == b.vendor_id) && (a.product_id == b.product_id);
}

#endif
