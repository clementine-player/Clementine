#ifndef ILISTER_H
#define ILISTER_H

#include "devicelister.h"

#include <libimobiledevice/afc.h>
#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/lockdown.h>

#include <QMutex>

class iLister : public DeviceLister {
  Q_OBJECT
 public:
  iLister();
  ~iLister();

  virtual QStringList DeviceUniqueIDs();
  virtual QStringList DeviceIcons(const QString& id);
  virtual QString DeviceManufacturer(const QString& id);
  virtual QString DeviceModel(const QString& id);
  virtual quint64 DeviceCapacity(const QString& id);
  virtual quint64 DeviceFreeSpace(const QString& id);
  virtual QVariantMap DeviceHardwareInfo(const QString& id);
  virtual QString MakeFriendlyName(const QString& id);
  virtual QUrl MakeDeviceUrl(const QString& id);
  virtual void UnmountDevice(const QString& id);

 private:
  struct DeviceInfo {
    DeviceInfo() : free_bytes(0), total_bytes(0) {}

    QString uuid;
    QString product_type;
    quint64 free_bytes;
    quint64 total_bytes;
  };

  class Connection {
   public:
    explicit Connection(const char* uuid);
    ~Connection();

    QString GetProperty(const char* property);
    quint64 GetInfoLongLong(const char* key);

   private:
    idevice_t device_;
    lockdownd_client_t lockdown_;
    afc_client_t afc_;

    uint16_t afc_port_;
  };

  virtual void Init();

  static void EventCallback(const idevice_event_t* event, void* context);

  void DeviceAddedCallback(const char* uuid);
  void DeviceRemovedCallback(const char* uuid);

  DeviceInfo ReadDeviceInfo(const char* uuid);
  static QString UniqueId(const char* uuid);

  template <typename T>
  T LockAndGetDeviceInfo(const QString& id, T DeviceInfo::*field);

private:
  QMutex mutex_;
  QMap<QString, DeviceInfo> devices_;
};

template <typename T>
T iLister::LockAndGetDeviceInfo(const QString& id, T DeviceInfo::*field) {
  QMutexLocker l(&mutex_);
  if (!devices_.contains(id))
    return T();

  return devices_[id].*field;
}

#endif
