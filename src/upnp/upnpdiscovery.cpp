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

#include "upnpdiscovery.h"

#include <QNetworkAccessManager>
#include <QNetworkDatagram>
#include <QNetworkInterface>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QUdpSocket>

#include "core/logging.h"
#include "upnpsoap.h"

const int UpnpDiscovery::kSearchIntervalMsec = 30000;
const int UpnpDiscovery::kLostAfterMsec = 3 * kSearchIntervalMsec + 5000;

namespace {

const char kSsdpAddress[] = "239.255.255.250";
const quint16 kSsdpPort = 1900;
// Sent more than once a search, in case a datagram is lost.
const int kSearchRepeats = 2;
const int kDescriptionTimeoutMsec = 5000;
// Descriptions are a few kilobytes.
const qint64 kMaxDescriptionBytes = 256 * 1024;

}  // namespace

UpnpDiscovery::UpnpDiscovery(QObject* parent)
    : QObject(parent),
      socket_(new QUdpSocket(this)),
      network_(new QNetworkAccessManager(this)),
      search_timer_(new QTimer(this)) {
  search_timer_->setInterval(kSearchIntervalMsec);
  connect(search_timer_, &QTimer::timeout, this, &UpnpDiscovery::Search);
  connect(socket_, &QUdpSocket::readyRead, this, &UpnpDiscovery::ReadDatagrams);
}

QByteArray UpnpDiscovery::SearchMessage() {
  return QByteArray(
      "M-SEARCH * HTTP/1.1\r\n"
      "HOST: 239.255.255.250:1900\r\n"
      "MAN: \"ssdp:discover\"\r\n"
      "MX: 1\r\n"
      "ST: urn:schemas-upnp-org:device:MediaRenderer:1\r\n"
      "\r\n");
}

void UpnpDiscovery::Start() {
  if (socket_->state() == QAbstractSocket::BoundState) return;
  // Responses come back to the port the search was sent from.
  if (!socket_->bind(QHostAddress::AnyIPv4, 0)) {
    qLog(Warning) << "Can't look for UPnP renderers:" << socket_->errorString();
    return;
  }
  socket_->setSocketOption(QAbstractSocket::MulticastTtlOption, 2);
  search_timer_->start();
  Search();
}

void UpnpDiscovery::Search() {
  // On every network, not just the one with the default route.
  const QByteArray message = SearchMessage();
  for (const QNetworkInterface& iface : QNetworkInterface::allInterfaces()) {
    const QNetworkInterface::InterfaceFlags flags = iface.flags();
    if (!(flags & QNetworkInterface::IsUp) ||
        !(flags & QNetworkInterface::IsRunning) ||
        !(flags & QNetworkInterface::CanMulticast) ||
        (flags & QNetworkInterface::IsLoopBack)) {
      continue;
    }
    bool has_ipv4 = false;
    for (const QNetworkAddressEntry& entry : iface.addressEntries()) {
      if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
        has_ipv4 = true;
      }
    }
    if (!has_ipv4) continue;

    socket_->setMulticastInterface(iface);
    for (int i = 0; i < kSearchRepeats; ++i) {
      socket_->writeDatagram(message, QHostAddress(kSsdpAddress), kSsdpPort);
    }
  }

  // Renderers that stopped answering.
  bool changed = false;
  for (auto it = renderers_.begin(); it != renderers_.end();) {
    if (it->last_seen.hasExpired(kLostAfterMsec)) {
      sonos_groups_.remove(it->device.root_id);
      it = renderers_.erase(it);
      changed = true;
    } else {
      ++it;
    }
  }
  if (changed) Update();
}

void UpnpDiscovery::ReadDatagrams() {
  while (socket_->hasPendingDatagrams()) {
    const QByteArray datagram = socket_->receiveDatagram().data();
    UpnpSsdpResponse response;
    if (response.Parse(datagram)) ResponseReceived(response);
  }
}

void UpnpDiscovery::ResponseReceived(const UpnpSsdpResponse& response) {
  Renderer& renderer = renderers_[response.id];
  renderer.last_seen.start();

  if (renderer.location != response.location) {
    // New, or moved: describe it again.
    renderer.location = response.location;
    renderer.ready = false;
    renderer.ignored = false;
    renderer.fetching = false;
  }
  if (renderer.ignored || renderer.fetching) return;
  if (!renderer.ready) {
    FetchDescription(response.id);
    return;
  }

  // Once a search: groups change without the players' descriptions doing so.
  if (renderer.device.zone_group_topology.is_valid() &&
      !renderer.topology_pending &&
      (!renderer.topology_requested.isValid() ||
       renderer.topology_requested.hasExpired(kSearchIntervalMsec / 2))) {
    FetchTopology(response.id);
  }
}

void UpnpDiscovery::FetchDescription(const QString& id) {
  Renderer& renderer = renderers_[id];
  renderer.fetching = true;
  const QUrl location = renderer.location;

  QNetworkRequest request(location);
  request.setTransferTimeout(kDescriptionTimeoutMsec);
  QNetworkReply* reply = network_->get(request);
  connect(reply, &QNetworkReply::finished, this, [this, reply, id, location]() {
    reply->deleteLater();
    auto it = renderers_.find(id);
    if (it == renderers_.end() || it->location != location) return;
    it->fetching = false;

    if (reply->error() != QNetworkReply::NoError) {
      // The next search tries again.
      qLog(Debug) << "No UPnP description from" << location << ":"
                  << reply->errorString();
      return;
    }
    UpnpDevice device;
    if (!device.ParseDescription(reply->read(kMaxDescriptionBytes), location)) {
      qLog(Debug) << "Not a renderer that can play URLs:" << location;
      it->ignored = true;
      return;
    }
    it->device = device;
    FetchProtocolInfo(id);
  });
}

void UpnpDiscovery::FetchProtocolInfo(const QString& id) {
  Renderer& renderer = renderers_[id];
  renderer.fetching = true;
  const QUrl location = renderer.location;

  UpnpSoap::Action action;
  action.control_url = renderer.device.connection_manager.control_url;
  action.service_type = renderer.device.connection_manager.type;
  action.name = "GetProtocolInfo";
  if (!renderer.device.connection_manager.is_valid()) {
    // Every MediaRenderer should have one, and every one plays MP3.
    qLog(Warning) << renderer.device.name << "has no ConnectionManager";
    renderer.device.SetProtocolInfo("http-get:*:audio/mpeg:*");
    renderer.fetching = false;
    renderer.ready = true;
    Update();
    return;
  }

  QNetworkReply* reply = UpnpSoap::Send(network_, action);
  connect(reply, &QNetworkReply::finished, this, [this, reply, id, location]() {
    reply->deleteLater();
    auto it = renderers_.find(id);
    if (it == renderers_.end() || it->location != location) return;
    it->fetching = false;

    const UpnpSoap::Response response = UpnpSoap::ParseReply(reply);
    if (!response.ok) {
      qLog(Debug) << "No protocol info from" << it->device.name << ":"
                  << response.error;
      return;
    }
    it->device.SetProtocolInfo(response.values.value("Sink"));
    if (it->device.caps.formats.isEmpty()) {
      qLog(Info) << it->device.name << "plays nothing Clementine can send it";
      it->ignored = true;
      return;
    }
    it->ready = true;
    if (it->device.zone_group_topology.is_valid()) {
      FetchTopology(id);
    } else {
      Update();
    }
  });
}

void UpnpDiscovery::FetchTopology(const QString& id) {
  Renderer& renderer = renderers_[id];
  renderer.topology_pending = true;
  renderer.topology_requested.start();

  UpnpSoap::Action action;
  action.control_url = renderer.device.zone_group_topology.control_url;
  action.service_type = renderer.device.zone_group_topology.type;
  action.name = "GetZoneGroupState";
  QNetworkReply* reply = UpnpSoap::Send(network_, action);
  connect(reply, &QNetworkReply::finished, this, [this, reply, id]() {
    reply->deleteLater();
    auto it = renderers_.find(id);
    if (it != renderers_.end()) it->topology_pending = false;

    const UpnpSoap::Response response = UpnpSoap::ParseReply(reply);
    if (!response.ok) {
      // Another player may answer after the next search.
      qLog(Debug) << "No Sonos topology from" << reply->url().host() << ":"
                  << response.error;
      return;
    }
    TopologyReceived(
        SonosZoneGroup::ParseState(response.values.value("ZoneGroupState")));
  });
}

void UpnpDiscovery::TopologyReceived(const QList<SonosZoneGroup>& groups) {
  for (const SonosZoneGroup& group : groups) {
    const QStringList rooms = group.Rooms();
    for (const SonosZoneGroup::Member& member : group.members) {
      sonos_groups_[member.id] =
          member.id == group.coordinator_id ? rooms : QStringList();
    }
  }
  Update();
}

void UpnpDiscovery::Update() {
  QMap<QString, UpnpDevice> offered;
  for (auto it = renderers_.constBegin(); it != renderers_.constEnd(); ++it) {
    if (!it->ready) continue;
    UpnpDevice device = it->device;
    if (device.zone_group_topology.is_valid()) {
      // Not until it's known to be a coordinator.
      const QStringList rooms = sonos_groups_.value(device.root_id);
      if (rooms.isEmpty()) continue;
      device.name = rooms.join(" + ");
      device.group = rooms.size() > 1;
    }
    offered[device.id] = device;
  }

  for (const UpnpDevice& device : offered) {
    auto it = offered_.find(device.id);
    if (it != offered_.end() && *it == device) continue;
    qLog(Info) << "Found UPnP renderer" << device << "playing"
               << device.caps.mime_types();
    emit DeviceFound(device);
  }
  for (const UpnpDevice& device : offered_) {
    if (offered.contains(device.id)) continue;
    qLog(Info) << "Lost UPnP renderer" << device;
    emit DeviceLost(device.id);
  }
  offered_ = offered;
}
