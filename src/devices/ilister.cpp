#include "ilister.h"
#include "imobiledeviceconnection.h"

#include <QStringList>
#include <QtDebug>

iLister::iLister() {
}

iLister::~iLister() {
}

void iLister::Init() {
  idevice_event_subscribe(&EventCallback, reinterpret_cast<void*>(this));
}

void iLister::EventCallback(const idevice_event_t* event, void* context) {
  qDebug() << Q_FUNC_INFO;

  iLister* me = reinterpret_cast<iLister*>(context);

  const char* uuid = event->uuid;

  switch (event->event) {
    case IDEVICE_DEVICE_ADD:
      me->DeviceAddedCallback(uuid);
      break;

    case IDEVICE_DEVICE_REMOVE:
      me->DeviceRemovedCallback(uuid);
      break;
  }
}


void iLister::DeviceAddedCallback(const char* uuid) {
  qDebug() << Q_FUNC_INFO;

  DeviceInfo info = ReadDeviceInfo(uuid);
  QString id = UniqueId(uuid);

  {
    QMutexLocker l(&mutex_);
    devices_[id] = info;
  }

  emit DeviceAdded(id);
}

void iLister::DeviceRemovedCallback(const char* uuid) {
  qDebug() << Q_FUNC_INFO;

  QString id = UniqueId(uuid);
  {
    QMutexLocker l(&mutex_);
    if (!devices_.contains(id))
      return;

    devices_.remove(id);
  }

  emit DeviceRemoved(id);
}

QString iLister::UniqueId(const char *uuid) {
  return "ithing/" + QString::fromUtf8(uuid);
}

QStringList iLister::DeviceUniqueIDs() {
  return devices_.keys();
}

QStringList iLister::DeviceIcons(const QString& id) {
  return QStringList();
}

QString iLister::DeviceManufacturer(const QString& id) {
  return "Apple";
}

QString iLister::DeviceModel(const QString& id) {
  return LockAndGetDeviceInfo(id, &DeviceInfo::product_type);
}

quint64 iLister::DeviceCapacity(const QString& id) {
  return LockAndGetDeviceInfo(id, &DeviceInfo::total_bytes);
}

quint64 iLister::DeviceFreeSpace(const QString& id) {
  return LockAndGetDeviceInfo(id, &DeviceInfo::free_bytes);
}

QVariantMap iLister::DeviceHardwareInfo(const QString& id) { return QVariantMap(); }

QString iLister::MakeFriendlyName(const QString& id) {
  QString model_id = LockAndGetDeviceInfo(id, &DeviceInfo::product_type);

  if (model_id.startsWith("iPhone")) {
    QString version = model_id.right(3);
    QChar major = version[0];
    QChar minor = version[2];
    if (major == '1' && minor == '1') {
      return "iPhone";
    }
    if (major == '1' && minor == '2') {
      return "iPhone 3G";
    }
    if (major == '2' && minor == '1') {
      return "iPhone 3GS";
    }
    if (major == '3' && minor == '1') {
      return "iPhone 4";
    }
  } else if (model_id.startsWith("iPod")) {
    return "iPod Touch";
  } else if (model_id.startsWith("iPad")) {
    return "iPad";
  }
  return model_id;
}

QList<QUrl> iLister::MakeDeviceUrls(const QString& id) {
  QList<QUrl> ret;

  QString uuid = LockAndGetDeviceInfo(id, &DeviceInfo::uuid);
  if (uuid.isEmpty())
    return ret;

  ret << QUrl("afc://" + uuid + "/");

  return ret;
}

void iLister::UnmountDevice(const QString& id) { }

iLister::DeviceInfo iLister::ReadDeviceInfo(const char* uuid) {
  DeviceInfo ret;

  iMobileDeviceConnection conn(uuid);
  ret.uuid = uuid;
  ret.product_type = conn.GetProperty("ProductType");
  ret.free_bytes = conn.GetInfoLongLong("FSFreeBytes");
  ret.total_bytes = conn.GetInfoLongLong("FSTotalBytes");

  return ret;
}
