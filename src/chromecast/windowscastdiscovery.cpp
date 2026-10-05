/* This file is part of Clementine.
   Copyright 2026, John Maguire <john.maguire@gmail.com>

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

#include "windowscastdiscovery.h"

// windns.h needs windows.h first.
// clang-format off
#include <windows.h>
#include <windns.h>
// clang-format on

#include <QMetaObject>
#include <QMutex>
#include <QMutexLocker>
#include <QtEndian>
#include <functional>

#include "core/logging.h"

namespace {

// Loaded at runtime, like WindowsDnsSd's, so a missing function only stops
// discovery rather than Clementine starting.
struct DnsSdBrowseApi {
  decltype(&::DnsServiceBrowse) browse = nullptr;
  decltype(&::DnsServiceBrowseCancel) browse_cancel = nullptr;
  decltype(&::DnsServiceResolve) resolve = nullptr;
  decltype(&::DnsServiceFreeInstance) free_instance = nullptr;
  decltype(&::DnsFree) free = nullptr;

  bool loaded() const {
    return browse && browse_cancel && resolve && free_instance && free;
  }
};

const DnsSdBrowseApi& Api() {
  static const DnsSdBrowseApi api = []() {
    DnsSdBrowseApi api;
    HMODULE dnsapi = LoadLibraryW(L"dnsapi.dll");
    if (!dnsapi) return api;
    api.browse = reinterpret_cast<decltype(api.browse)>(
        GetProcAddress(dnsapi, "DnsServiceBrowse"));
    api.browse_cancel = reinterpret_cast<decltype(api.browse_cancel)>(
        GetProcAddress(dnsapi, "DnsServiceBrowseCancel"));
    api.resolve = reinterpret_cast<decltype(api.resolve)>(
        GetProcAddress(dnsapi, "DnsServiceResolve"));
    api.free_instance = reinterpret_cast<decltype(api.free_instance)>(
        GetProcAddress(dnsapi, "DnsServiceFreeInstance"));
    api.free =
        reinterpret_cast<decltype(api.free)>(GetProcAddress(dnsapi, "DnsFree"));
    return api;
  }();
  return api;
}

QString FromWide(PCWSTR text) {
  return text ? QString::fromWCharArray(text) : QString();
}

}  // namespace

// What Windows' callbacks need to reach the discovery. |owner| is cleared,
// under the lock, when the discovery is destroyed.
struct WindowsCastDiscovery::Shared {
  QMutex mutex;
  WindowsCastDiscovery* owner = nullptr;

  // Runs |f| on the main thread, unless the discovery is gone by then.
  void Post(std::function<void(WindowsCastDiscovery*)> f) {
    QMutexLocker l(&mutex);
    if (!owner) return;
    WindowsCastDiscovery* o = owner;
    // A call queued for an object that's then destroyed is dropped.
    QMetaObject::invokeMethod(o, [o, f]() { f(o); }, Qt::QueuedConnection);
  }
};

struct WindowsCastDiscovery::Browse {
  std::wstring query;
  DNS_SERVICE_BROWSE_REQUEST request = {};
  DNS_SERVICE_CANCEL cancel = {};

  static VOID WINAPI Callback(DWORD status, PVOID context,
                              PDNS_RECORD records) {
    Shared* shared = static_cast<Shared*>(context);
    if (status != ERROR_SUCCESS) {
      if (records) Api().free(records, DnsFreeRecordList);
      if (status == ERROR_CANCELLED) return;
      shared->Post([status](WindowsCastDiscovery*) {
        qLog(Warning) << "Windows stopped looking for Cast devices, error"
                      << status;
      });
      return;
    }

    // The API only takes wide strings, so the records are DNS_RECORDW.
    for (DNS_RECORDW* r = reinterpret_cast<DNS_RECORDW*>(records); r;
         r = r->pNext) {
      if (r->wType != DNS_TYPE_PTR) continue;
      const QString instance = FromWide(r->Data.PTR.pNameHost);
      // A goodbye is the same record with a TTL of 0.
      const bool removed = r->dwTtl == 0;
      shared->Post([instance, removed](WindowsCastDiscovery* self) {
        self->PtrRecord(instance, removed);
      });
    }
    if (records) Api().free(records, DnsFreeRecordList);
  }
};

// One resolve, freed by its completion callback.
struct WindowsCastDiscovery::Resolve {
  Shared* shared;
  QString instance;
  std::wstring query;
  DNS_SERVICE_RESOLVE_REQUEST request = {};
  DNS_SERVICE_CANCEL cancel = {};

  static VOID WINAPI Callback(DWORD status, PVOID context,
                              PDNS_SERVICE_INSTANCE result) {
    std::unique_ptr<Resolve> self(static_cast<Resolve*>(context));
    const QString instance = self->instance;

    if (status != ERROR_SUCCESS || !result) {
      if (result) Api().free_instance(result);
      self->shared->Post([instance, status](WindowsCastDiscovery*) {
        qLog(Debug) << "Couldn't resolve Cast service" << instance << ", error"
                    << status;
      });
      return;
    }

    QList<QByteArray> txt;
    for (DWORD i = 0; i < result->dwPropertyCount; ++i) {
      txt << (FromWide(result->keys[i]) + "=" + FromWide(result->values[i]))
                 .toUtf8();
    }
    CastDevice device;
    const bool ok = device.ParseTxt(txt);
    device.port = result->wPort;
    if (result->ip4Address) {
      // In network byte order.
      device.address = QHostAddress(
          qFromBigEndian<quint32>(static_cast<quint32>(*result->ip4Address)));
    }
    const QString host = FromWide(result->pszHostName);
    Api().free_instance(result);

    self->shared->Post([instance, ok, device, host](WindowsCastDiscovery* d) {
      if (!ok) {
        qLog(Debug) << "Cast service" << instance << "has no id";
      } else if (device.address.isNull()) {
        d->ResolveHost(instance, device, host);
      } else {
        d->ServiceResolved(instance, device);
      }
    });
  }
};

WindowsCastDiscovery::WindowsCastDiscovery(QObject* parent)
    : CastDiscovery(parent), shared_(new Shared) {
  shared_->owner = this;
}

WindowsCastDiscovery::~WindowsCastDiscovery() {
  {
    QMutexLocker l(&shared_->mutex);
    shared_->owner = nullptr;
  }
  if (browse_) {
    Api().browse_cancel(&browse_->cancel);
    // Windows may still be reading the request while it cancels, so it's
    // left allocated.
    (void)browse_.release();
  }
  // Windows' callbacks may also still run, and they only hold |shared_|, so
  // that's left allocated too. There's one of each per discovery.
}

void WindowsCastDiscovery::Start() {
  if (browse_) return;
  if (!Api().loaded()) {
    qLog(Warning) << "This version of Windows can't look for Cast devices";
    return;
  }

  browse_.reset(new Browse);
  browse_->query = QString("%1.local").arg(kServiceType).toStdWString();
  browse_->request.Version = DNS_QUERY_REQUEST_VERSION1;
  browse_->request.InterfaceIndex = 0;
  browse_->request.QueryName = browse_->query.c_str();
  browse_->request.pBrowseCallback = &Browse::Callback;
  browse_->request.pQueryContext = shared_;

  const DNS_STATUS status = Api().browse(&browse_->request, &browse_->cancel);
  if (status != DNS_REQUEST_PENDING) {
    qLog(Warning) << "Couldn't look for Cast devices, error" << status;
    browse_.reset();
    return;
  }
  qLog(Debug) << "Looking for Cast devices";
}

void WindowsCastDiscovery::PtrRecord(const QString& instance, bool removed) {
  // DNS names are case-insensitive.
  const QString service = instance.toLower();
  if (removed) {
    ServiceRemoved(service);
    return;
  }
  // Windows reports a service again every time it's announced.
  if (IsLive(service)) return;
  ServiceAdded(service);
  StartResolve(instance);
}

void WindowsCastDiscovery::StartResolve(const QString& instance) {
  std::unique_ptr<Resolve> resolve(new Resolve);
  resolve->shared = shared_;
  resolve->instance = instance.toLower();
  resolve->query = instance.toStdWString();
  resolve->request.Version = DNS_QUERY_REQUEST_VERSION1;
  resolve->request.InterfaceIndex = 0;
  resolve->request.QueryName = const_cast<PWSTR>(resolve->query.c_str());
  resolve->request.pResolveCompletionCallback = &Resolve::Callback;
  resolve->request.pQueryContext = resolve.get();

  const DNS_STATUS status = Api().resolve(&resolve->request, &resolve->cancel);
  if (status != DNS_REQUEST_PENDING) {
    qLog(Debug) << "Couldn't resolve Cast service" << instance << ", error"
                << status;
    return;
  }
  // The callback owns it now.
  resolve.release();
}
