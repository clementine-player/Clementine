#include "windowsdnssd.h"

// windns.h needs windows.h first.
// clang-format off
#include <windows.h>
#include <windns.h>
// clang-format on

#include <QHostInfo>
#include <QMap>
#include <QMutex>
#include <QMutexLocker>
#include <QtEndian>
#include <memory>

#include "core/logging.h"

namespace {

// Loaded at runtime so Clementine still starts on versions of Windows older
// than 10 1809, which don't have them.
struct DnsSdApi {
  decltype(&::DnsServiceConstructInstance) construct_instance = nullptr;
  decltype(&::DnsServiceFreeInstance) free_instance = nullptr;
  decltype(&::DnsServiceRegister) register_service = nullptr;
  decltype(&::DnsServiceDeRegister) deregister_service = nullptr;

  bool loaded() const {
    return construct_instance && free_instance && register_service &&
           deregister_service;
  }
};

const DnsSdApi& Api() {
  static const DnsSdApi api = []() {
    DnsSdApi api;
    HMODULE dnsapi = LoadLibraryW(L"dnsapi.dll");
    if (!dnsapi) return api;
    api.construct_instance = reinterpret_cast<decltype(api.construct_instance)>(
        GetProcAddress(dnsapi, "DnsServiceConstructInstance"));
    api.free_instance = reinterpret_cast<decltype(api.free_instance)>(
        GetProcAddress(dnsapi, "DnsServiceFreeInstance"));
    api.register_service = reinterpret_cast<decltype(api.register_service)>(
        GetProcAddress(dnsapi, "DnsServiceRegister"));
    api.deregister_service = reinterpret_cast<decltype(api.deregister_service)>(
        GetProcAddress(dnsapi, "DnsServiceDeRegister"));
    return api;
  }();
  return api;
}

// A DNS-SD instance name is a single label, so dots and backslashes in it
// have to be escaped.
QString EscapeInstanceLabel(const QString& name) {
  QString escaped;
  for (QChar c : name) {
    if (c == '.' || c == '\\') escaped += '\\';
    escaped += c;
  }
  return escaped;
}

}  // namespace

// Windows reads the request and instance until the registration is
// withdrawn, so they live here until then.
struct WindowsDnsSd::Registration {
  enum class State {
    Registering,
    Registered,
    Failed,
    Deregistering,
    Done,
  };

  QMutex mutex;
  State state = State::Registering;
  // Coordinate deletion across threads: Windows may read &request until both
  // deregister_service returns and the completion callback finishes.
  bool deregister_requested = false;
  bool deregister_returned = false;
  bool deregister_callback_done = false;
  DNS_SERVICE_REGISTER_REQUEST request = {};
  PDNS_SERVICE_INSTANCE instance = nullptr;
  IP4_ADDRESS ipv4 = 0;
  IP6_ADDRESS ipv6 = {};

  static VOID WINAPI Completed(DWORD status, PVOID context,
                               PDNS_SERVICE_INSTANCE instance) {
    // Windows hands over a copy of the instance, which is ours to free.
    if (instance) Api().free_instance(instance);

    Registration* self = static_cast<Registration*>(context);
    bool call_deregister = false;
    bool should_delete = false;
    {
      QMutexLocker l(&self->mutex);
      if (self->state == State::Registering) {
        if (status == ERROR_SUCCESS) {
          if (self->deregister_requested) {
            self->state = State::Deregistering;
            self->deregister_returned = false;
            self->deregister_callback_done = false;
            call_deregister = true;
          } else {
            self->state = State::Registered;
          }
        } else {
          qLog(Warning) << "Windows couldn't advertise the remote, error"
                        << status;
          self->state = State::Failed;
          if (self->deregister_requested) {
            self->state = State::Done;
            should_delete = true;
          }
        }
      } else if (self->state == State::Deregistering) {
        self->state = State::Done;
        self->deregister_callback_done = true;
        should_delete = self->deregister_returned;
      }
    }

    // Call deregister_service outside mutex so a synchronous or re-entrant
    // completion callback cannot deadlock on self->mutex.
    if (call_deregister) {
      const DWORD err = Api().deregister_service(&self->request, nullptr);
      {
        QMutexLocker l(&self->mutex);
        self->deregister_returned = true;
        should_delete =
            (err != DNS_REQUEST_PENDING) || self->deregister_callback_done;
      }
    }
    if (should_delete) {
      delete self;
    }
  }

  ~Registration() {
    if (instance) Api().free_instance(instance);
  }
};

WindowsDnsSd::WindowsDnsSd() {}

WindowsDnsSd::~WindowsDnsSd() { UnpublishInternal(); }

bool WindowsDnsSd::IsAvailable() { return Api().loaded(); }

void WindowsDnsSd::PublishInternal(const QString& domain, const QString& type,
                                   const QByteArray& name, quint16 port,
                                   const QList<QHostAddress>& addresses) {
  const QString instance_name =
      QString("%1.%2.%3")
          .arg(EscapeInstanceLabel(QString::fromUtf8(name.constData())), type,
               domain);
  const QString host = QString("%1.%2").arg(QHostInfo::localHostName(), domain);

  bool ok = true;
  if (addresses.isEmpty()) {
    ok = Register(instance_name, host, port, 0, addresses);
  } else {
    // Windows registers a service on one interface or on all of them, so
    // advertise on each interface the chosen addresses are on.
    QMap<int, QList<QHostAddress>> by_interface;
    for (const QHostAddress& address : addresses) {
      const int iface = InterfaceIndexOf(address);
      if (iface < 0) {
        qLog(Warning) << "Not advertising the remote on" << address.toString()
                      << "- it isn't on any network interface";
        continue;
      }
      by_interface[iface] << address;
    }
    for (auto it = by_interface.begin(); ok && it != by_interface.end(); ++it) {
      ok = Register(instance_name, host, port, it.key(), it.value());
    }
  }
  if (ok) return;

  // Remotes can still connect by address.
  qLog(Warning) << "Couldn't advertise the remote";
  DeregisterAll();
}

bool WindowsDnsSd::Register(const QString& instance_name, const QString& host,
                            quint16 port, int iface,
                            const QList<QHostAddress>& addresses) {
  std::unique_ptr<Registration> registration(new Registration);

  PIP4_ADDRESS ipv4 = nullptr;
  PIP6_ADDRESS ipv6 = nullptr;
  for (const QHostAddress& address : addresses) {
    if (!ipv4 && address.protocol() == QAbstractSocket::IPv4Protocol) {
      registration->ipv4 = qToBigEndian(address.toIPv4Address());
      ipv4 = &registration->ipv4;
    } else if (!ipv6 && address.protocol() == QAbstractSocket::IPv6Protocol) {
      const Q_IPV6ADDR bytes = address.toIPv6Address();
      memcpy(registration->ipv6.IP6Byte, bytes.c, sizeof(bytes.c));
      ipv6 = &registration->ipv6;
    }
  }

  registration->instance =
      Api().construct_instance(reinterpret_cast<PCWSTR>(instance_name.utf16()),
                               reinterpret_cast<PCWSTR>(host.utf16()), ipv4,
                               ipv6, port, 0, 0, 0, nullptr, nullptr);
  if (!registration->instance) {
    qLog(Warning) << "Couldn't describe the remote's service to Windows";
    return false;
  }

  DNS_SERVICE_REGISTER_REQUEST* request = &registration->request;
  request->Version = DNS_QUERY_REQUEST_VERSION1;
  request->InterfaceIndex = iface;
  request->pServiceInstance = registration->instance;
  request->pRegisterCompletionCallback = &Registration::Completed;
  request->pQueryContext = registration.get();

  // Hold the registration before calling register_service so an immediate
  // callback on another thread cannot race ahead of insertion.
  Registration* raw = registration.release();
  registrations_ << raw;

  const DWORD status = Api().register_service(&raw->request, nullptr);
  if (status != DNS_REQUEST_PENDING) {
    qLog(Warning) << "Windows refused to advertise the remote, error" << status;
    registrations_.removeOne(raw);
    delete raw;
    return false;
  }

  qLog(Debug) << "Publishing" << instance_name << "on interface" << iface;
  return true;
}

void WindowsDnsSd::UnpublishInternal() { DeregisterAll(); }

void WindowsDnsSd::DeregisterAll() {
  const QList<Registration*> pending = registrations_;
  registrations_.clear();

  for (Registration* registration : pending) {
    bool call_deregister = false;
    bool should_delete = false;
    {
      QMutexLocker l(&registration->mutex);
      switch (registration->state) {
        case Registration::State::Registering:
          // Defer deregistration (or deletion if registration fails) to the
          // completion callback so we never deregister a failed or in-flight
          // registration or block the NetworkRemote thread.
          registration->deregister_requested = true;
          break;
        case Registration::State::Registered:
          registration->state = Registration::State::Deregistering;
          registration->deregister_returned = false;
          registration->deregister_callback_done = false;
          call_deregister = true;
          break;
        case Registration::State::Failed:
        case Registration::State::Done:
          registration->state = Registration::State::Done;
          should_delete = true;
          break;
        case Registration::State::Deregistering:
          break;
      }
    }

    // Invoke deregister_service unlocked, then free only after both this
    // call returns and any pending completion callback has fired.
    if (call_deregister) {
      const DWORD err =
          Api().deregister_service(&registration->request, nullptr);
      {
        QMutexLocker l(&registration->mutex);
        registration->deregister_returned = true;
        should_delete = (err != DNS_REQUEST_PENDING) ||
                        registration->deregister_callback_done;
      }
    }
    if (should_delete) {
      delete registration;
    }
  }
}
