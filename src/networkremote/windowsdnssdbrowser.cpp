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

#include "windowsdnssdbrowser.h"

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

// What Windows' callbacks need to reach the browser. |owner| is cleared,
// under the lock, when the browser is destroyed.
struct WindowsDnsSdBrowser::Shared {
  QMutex mutex;
  WindowsDnsSdBrowser* owner = nullptr;
  QString service_type;

  // Runs |f| on the main thread, unless the browser is gone by then.
  void Post(std::function<void(WindowsDnsSdBrowser*)> f) {
    QMutexLocker l(&mutex);
    if (!owner) return;
    WindowsDnsSdBrowser* o = owner;
    // A call queued for an object that's then destroyed is dropped.
    QMetaObject::invokeMethod(o, [o, f]() { f(o); }, Qt::QueuedConnection);
  }
};

struct WindowsDnsSdBrowser::Browse {
  std::wstring query;
  DNS_SERVICE_BROWSE_REQUEST request = {};
  DNS_SERVICE_CANCEL cancel = {};

  static VOID WINAPI Callback(DWORD status, PVOID context,
                              PDNS_RECORD records) {
    Shared* shared = static_cast<Shared*>(context);
    if (status != ERROR_SUCCESS) {
      if (records) Api().free(records, DnsFreeRecordList);
      if (status == ERROR_CANCELLED) return;
      shared->Post([status](WindowsDnsSdBrowser* self) {
        qLog(Warning) << "Windows stopped looking for" << self->service_type_
                      << "services, error" << status;
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
      shared->Post([instance, removed](WindowsDnsSdBrowser* self) {
        self->PtrRecord(instance, removed);
      });
    }
    if (records) Api().free(records, DnsFreeRecordList);
  }
};

// One resolve, freed by its completion callback.
struct WindowsDnsSdBrowser::Resolve {
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
      self->shared->Post([instance, status](WindowsDnsSdBrowser*) {
        qLog(Debug) << "Couldn't resolve" << instance << ", error" << status;
      });
      return;
    }

    Service service;
    service.instance = FromWide(result->pszInstanceName);
    for (DWORD i = 0; i < result->dwPropertyCount; ++i) {
      service.txt << (FromWide(result->keys[i]) + "=" +
                      FromWide(result->values[i]))
                         .toUtf8();
    }
    service.port = result->wPort;
    if (result->ip4Address) {
      // In network byte order.
      service.address = QHostAddress(
          qFromBigEndian<quint32>(static_cast<quint32>(*result->ip4Address)));
    }
    service.host = FromWide(result->pszHostName);
    Api().free_instance(result);

    self->shared->Post([instance, service](WindowsDnsSdBrowser* browser) {
      if (!browser->live_.contains(instance)) return;
      emit browser->ServiceResolved(instance, service);
    });
  }
};

WindowsDnsSdBrowser::WindowsDnsSdBrowser(const QString& service_type,
                                         QObject* parent)
    : QObject(parent), service_type_(service_type), shared_(new Shared) {
  shared_->owner = this;
}

WindowsDnsSdBrowser::~WindowsDnsSdBrowser() {
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
  // that's left allocated too. There's one of each per browser.
}

bool WindowsDnsSdBrowser::Available() { return Api().loaded(); }

bool WindowsDnsSdBrowser::Start() {
  if (browse_) return true;
  if (!Available()) return false;

  browse_.reset(new Browse);
  browse_->query = QString("%1.local").arg(service_type_).toStdWString();
  browse_->request.Version = DNS_QUERY_REQUEST_VERSION1;
  browse_->request.InterfaceIndex = 0;
  browse_->request.QueryName = browse_->query.c_str();
  browse_->request.pBrowseCallback = &Browse::Callback;
  browse_->request.pQueryContext = shared_;

  const DNS_STATUS status = Api().browse(&browse_->request, &browse_->cancel);
  if (status != DNS_REQUEST_PENDING) {
    qLog(Warning) << "Couldn't look for" << service_type_ << "services, error"
                  << status;
    browse_.reset();
    return false;
  }
  return true;
}

void WindowsDnsSdBrowser::PtrRecord(const QString& instance, bool removed) {
  // DNS names are case-insensitive.
  const QString service = instance.toLower();
  if (removed) {
    if (live_.remove(service)) emit ServiceRemoved(service);
    return;
  }
  // Windows reports a service again every time it's announced.
  if (live_.contains(service)) return;
  live_ << service;
  emit ServiceAdded(service);
  StartResolve(instance);
}

void WindowsDnsSdBrowser::StartResolve(const QString& instance) {
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
    qLog(Debug) << "Couldn't resolve" << instance << ", error" << status;
    return;
  }
  // The callback owns it now.
  resolve.release();
}
