#include "windowsdnssd.h"

// windns.h needs windows.h first.
// clang-format off
#include <windows.h>
#include <windns.h>
// clang-format on

#include <QHostInfo>
#include <QMap>

#include "core/logging.h"
#include "tinysvcmdns.h"

namespace {

// How long to wait for Windows to finish a registration or deregistration.
// It probes the name on the network first, which takes about a second.
const DWORD kCompletionTimeoutMs = 5000;

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
  DNS_SERVICE_REGISTER_REQUEST request = {};
  PDNS_SERVICE_INSTANCE instance = nullptr;
  IP4_ADDRESS ipv4 = 0;
  IP6_ADDRESS ipv6 = {};

  // Signalled by Completed each time Windows finishes with the request.
  HANDLE completed = CreateEventW(nullptr, FALSE, FALSE, nullptr);
  DWORD status = ERROR_SUCCESS;
  // Whether Windows hadn't finished registering when we stopped waiting.
  bool pending = false;

  static VOID WINAPI Completed(DWORD status, PVOID context,
                               PDNS_SERVICE_INSTANCE instance) {
    Registration* self = static_cast<Registration*>(context);
    self->status = status;
    // Windows hands over a copy of the instance, which is ours to free.
    if (instance) Api().free_instance(instance);
    SetEvent(self->completed);
  }

  bool WaitForCompletion() {
    return WaitForSingleObject(completed, kCompletionTimeoutMs) ==
           WAIT_OBJECT_0;
  }

  ~Registration() {
    if (instance) Api().free_instance(instance);
    CloseHandle(completed);
  }
};

WindowsDnsSd::WindowsDnsSd() {}

WindowsDnsSd::~WindowsDnsSd() { UnpublishInternal(); }

bool WindowsDnsSd::IsAvailable() { return Api().loaded(); }

void WindowsDnsSd::PublishInternal(const QString& domain, const QString& type,
                                   const QByteArray& name, quint16 port,
                                   const QList<QHostAddress>& addresses) {
  if (!fallback_) {
    const QString instance_name =
        QString("%1.%2.%3")
            .arg(EscapeInstanceLabel(QString::fromUtf8(name.constData())), type,
                 domain);
    const QString host =
        QString("%1.%2").arg(QHostInfo::localHostName(), domain);

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
      for (auto it = by_interface.begin(); ok && it != by_interface.end();
           ++it) {
        ok = Register(instance_name, host, port, it.key(), it.value());
      }
    }
    if (ok) return;

    qLog(Warning) << "Falling back to tinysvcmdns to advertise the remote";
    DeregisterAll();
    fallback_.reset(new TinySVCMDNS);
  }

  fallback_->Publish(domain, type, QString::fromUtf8(name.constData()), port,
                     addresses);
}

bool WindowsDnsSd::Register(const QString& instance_name, const QString& host,
                            quint16 port, int iface,
                            const QList<QHostAddress>& addresses) {
  std::unique_ptr<Registration> registration(new Registration);

  PIP4_ADDRESS ipv4 = nullptr;
  PIP6_ADDRESS ipv6 = nullptr;
  for (const QHostAddress& address : addresses) {
    if (!ipv4 && address.protocol() == QAbstractSocket::IPv4Protocol) {
      registration->ipv4 = htonl(address.toIPv4Address());
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

  DNS_SERVICE_REGISTER_REQUEST& request = registration->request;
  request.Version = DNS_QUERY_REQUEST_VERSION1;
  request.InterfaceIndex = iface;
  request.pServiceInstance = registration->instance;
  request.pRegisterCompletionCallback = &Registration::Completed;
  request.pQueryContext = registration.get();

  DWORD status = Api().register_service(&request, nullptr);
  if (status != DNS_REQUEST_PENDING) {
    qLog(Warning) << "Windows refused to advertise the remote, error" << status;
    return false;
  }

  if (!registration->WaitForCompletion()) {
    // It may still finish, and Windows would then write to the
    // registration, so it has to outlive this.
    qLog(Warning) << "Windows is taking a while to advertise the remote";
    registration->pending = true;
  } else if (registration->status != ERROR_SUCCESS) {
    qLog(Warning) << "Windows couldn't advertise the remote, error"
                  << registration->status;
    return false;
  } else {
    qLog(Debug) << "Publishing" << instance_name << "on interface" << iface;
  }

  registrations_ << registration.release();
  return true;
}

void WindowsDnsSd::UnpublishInternal() {
  DeregisterAll();
  if (fallback_) fallback_->Unpublish();
}

void WindowsDnsSd::DeregisterAll() {
  for (Registration* registration : registrations_) {
    // Deregistering signals the same event, so first let an unfinished
    // registration finish.
    if (registration->pending && !registration->WaitForCompletion()) {
      qLog(Warning) << "Windows is still advertising the remote";
      continue;
    }
    const DWORD status =
        Api().deregister_service(&registration->request, nullptr);
    if (status == DNS_REQUEST_PENDING && !registration->WaitForCompletion()) {
      // Windows still has the registration, so it can't be freed.
      qLog(Warning) << "Windows is taking a while to stop advertising the"
                    << "remote";
      continue;
    }
    delete registration;
  }
  registrations_.clear();
}
