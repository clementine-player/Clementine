#include "ilister.h"

#include <QCoreApplication>
#include <QStringList>
#include <QtDebug>

#include <plist/plist.h>

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


iLister::Connection::Connection(const char* uuid)
    : device_(NULL), lockdown_(NULL), afc_(NULL), afc_port_(0) {
  idevice_error_t err = idevice_new(&device_, uuid);
  if (err != IDEVICE_E_SUCCESS) {
    qWarning() << "idevice error:" << err;
    return;
  }

  const char* label = QCoreApplication::applicationName().toUtf8().constData();
  lockdownd_error_t lockdown_err =
      lockdownd_client_new_with_handshake(device_, &lockdown_, label);
  if (lockdown_err != LOCKDOWN_E_SUCCESS) {
    qWarning() << "lockdown error:" << lockdown_err;
    return;
  }

  lockdown_err = lockdownd_start_service(lockdown_, "com.apple.afc", &afc_port_);
  if (lockdown_err != LOCKDOWN_E_SUCCESS) {
    qWarning() << "lockdown error:" << lockdown_err;
    return;
  }

  afc_error_t afc_err = afc_client_new(device_, afc_port_, &afc_);
  if (afc_err != 0) {
    qWarning() << "afc error:" << afc_err;
    return;
  }
}

iLister::Connection::~Connection() {
  if (afc_) {
    afc_client_free(afc_);
  }
  if (lockdown_) {
    lockdownd_client_free(lockdown_);
  }
  if (device_) {
    idevice_free(device_);
  }
}

QString iLister::Connection::GetProperty(const char* property) {
  plist_t node = NULL;
  lockdownd_get_value(lockdown_, NULL, property, &node);
  char* value = NULL;
  plist_get_string_val(node, &value);
  plist_free(node);

  QString ret = QString::fromUtf8(value);
  free(value);
  return ret;
}

quint64 iLister::Connection::GetInfoLongLong(const char* key) {
  char* value = NULL;
  afc_error_t err = afc_get_device_info_key(afc_, key, &value);
  if (err != AFC_E_SUCCESS || !value) {
    return 0;
  }
  QString num = QString::fromAscii(value);
  quint64 ret = num.toULongLong();
  free(value);
  return ret;
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

iLister::DeviceInfo iLister::ReadDeviceInfo(const char *uuid) {
  DeviceInfo ret;

  Connection conn(uuid);
  ret.uuid = uuid;
  ret.product_type = conn.GetProperty("ProductType");
  ret.free_bytes = conn.GetInfoLongLong("FSFreeBytes");
  ret.total_bytes = conn.GetInfoLongLong("FSTotalBytes");

  return ret;
}
