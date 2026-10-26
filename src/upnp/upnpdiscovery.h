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

#ifndef UPNP_UPNPDISCOVERY_H_
#define UPNP_UPNPDISCOVERY_H_

#include <QElapsedTimer>
#include <QMap>
#include <QObject>
#include <QString>

#include "sonostopology.h"
#include "upnpdevice.h"

class QNetworkAccessManager;
class QTimer;
class QUdpSocket;

// Finds UPnP MediaRenderers on the local network.
//
// They're found with SSDP, searching now and every kSearchIntervalMsec. Each
// new one's description says where its services are, and its
// ConnectionManager what it plays; DeviceFound comes once both are known,
// and again when something changes. A renderer that hasn't answered for
// kLostAfterMsec is lost.
//
// Sonos players are MediaRenderers too, but only a group's coordinator can
// be played on; see SonosZoneGroup. So a renderer whose root device has a
// ZoneGroupTopology is asked how it's grouped after each search, and only
// found while it's a coordinator, named after its group's rooms.
class UpnpDiscovery : public QObject {
  Q_OBJECT

 public:
  static const int kSearchIntervalMsec;
  static const int kLostAfterMsec;

  explicit UpnpDiscovery(QObject* parent = nullptr);

  void Start();

  QList<UpnpDevice> devices() const { return offered_.values(); }

  // The search sent to 239.255.255.250:1900.
  static QByteArray SearchMessage();

 signals:
  void DeviceFound(const UpnpDevice& device);
  void DeviceLost(const QString& id);

 private slots:
  void Search();
  void ReadDatagrams();

 private:
  struct Renderer {
    QUrl location;
    QElapsedTimer last_seen;
    bool fetching = false;
    // Described, and its protocol info known.
    bool ready = false;
    // Its description isn't a MediaRenderer's after all.
    bool ignored = false;
    UpnpDevice device;
    // Since the topology was last asked for; invalid if never.
    QElapsedTimer topology_requested;
    bool topology_pending = false;
  };

  void ResponseReceived(const UpnpSsdpResponse& response);
  void FetchDescription(const QString& id);
  void FetchProtocolInfo(const QString& id);
  void FetchTopology(const QString& id);
  void TopologyReceived(const QList<SonosZoneGroup>& groups);
  // Emits what's changed in what can be played on.
  void Update();

  QUdpSocket* socket_;
  QNetworkAccessManager* network_;
  QTimer* search_timer_;
  QMap<QString, Renderer> renderers_;
  // Sonos players' root ids, and whether each is its group's coordinator:
  // the group's name if so, otherwise empty.
  QMap<QString, QString> sonos_groups_;
  QMap<QString, UpnpDevice> offered_;
};

#endif  // UPNP_UPNPDISCOVERY_H_
