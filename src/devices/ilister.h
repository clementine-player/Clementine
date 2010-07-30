#ifndef ILISTER_H
#define ILISTER_H

#include "devicelister.h"

#include <libimobiledevice/afc.h>
#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/lockdown.h>

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
  virtual void Init();

  void DeviceAddedCallback(const char* uuid);
  void DeviceRemovedCallback(const char* uuid);

  static void EventCallback(const idevice_event_t* event, void* context);

  class Connection {
   public:
    explicit Connection(const char* uuid);
    ~Connection();

    QString GetProperty(const char* property);
    quint64 GetFreeBytes();
    quint64 GetTotalBytes();

   private:
    quint64 GetInfoLongLong(const char* key);

    idevice_t device_;
    lockdownd_client_t lockdown_;
    afc_client_t afc_;

    uint16_t afc_port_;
  };

  QMap<QString, Connection*> devices_;
};

#endif
