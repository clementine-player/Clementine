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

#ifndef UPNP_UPNPDEVICE_H_
#define UPNP_UPNPDEVICE_H_

#include <QByteArray>
#include <QHostAddress>
#include <QMap>
#include <QString>
#include <QUrl>

#include "networkremote/streaming/streamplanner.h"

class QDebug;

// One of a device's services, from its description.
struct UpnpService {
  // eg. "urn:schemas-upnp-org:service:AVTransport:1".
  QString type;
  QUrl control_url;
  // Where to subscribe to its events. Empty if it has none.
  QUrl event_url;

  bool is_valid() const { return !type.isEmpty() && control_url.isValid(); }
  bool operator==(const UpnpService& other) const {
    return type == other.type && control_url == other.control_url &&
           event_url == other.event_url;
  }
};

// A UPnP MediaRenderer that can be told to play a URL: one with an
// AVTransport.
struct UpnpDevice {
  // The MediaRenderer's UDN, without "uuid:". It stays the same when the
  // device restarts or changes address.
  QString id;
  // Its friendlyName, or what it's called in a group; see UpnpDiscovery.
  QString name;
  QString model;
  // Where its description is. Media is only served to this address.
  QHostAddress address;

  UpnpService av_transport;
  UpnpService rendering_control;
  UpnpService connection_manager;
  // Sonos players' volume for the whole group they're in.
  UpnpService group_rendering_control;

  // The root device's UDN, without "uuid:", which is the MediaRenderer's
  // when that is the root.
  QString root_id;
  // Sonos players say how they're grouped. See UpnpDiscovery.
  UpnpService zone_group_topology;

  // From ConnectionManager's GetProtocolInfo.
  RendererCaps caps;
  // The device's own name for each of caps' MIME types, eg. "audio/mp3" for
  // "audio/mpeg", which is what it's told it's being sent.
  QMap<QString, QString> device_mime_types;
  // Whether the Sink lists any video formats, which suggests it has a screen.
  bool video = false;
  // A Sonos group of more than one room. See UpnpDiscovery.
  bool group = false;
  // Whether it says it follows DLNA, as TVs do, with an X_DLNADOC.
  bool dlna = false;

  bool is_valid() const {
    return !id.isEmpty() && !address.isNull() && av_transport.is_valid();
  }

  // Fills in everything but the caps from a device description fetched from
  // |location|. Returns false if it doesn't describe a MediaRenderer with an
  // AVTransport.
  bool ParseDescription(const QByteArray& xml, const QUrl& location);
  // Fills in the caps from GetProtocolInfo's Sink: protocolInfo entries
  // separated by commas, eg. "http-get:*:audio/mpeg:*,...". Only those over
  // HTTP, in formats the planner knows, are used, but any video format over
  // HTTP sets video.
  void SetProtocolInfo(const QString& sink);

  // The MIME type to tell the device for |mime_type|, from caps.
  QString DeviceMimeType(const QString& mime_type) const;

  bool operator==(const UpnpDevice& other) const;
  bool operator!=(const UpnpDevice& other) const { return !(*this == other); }
};

QDebug operator<<(QDebug dbg, const UpnpDevice& device);

// A device's answer to an SSDP M-SEARCH for MediaRenderers.
struct UpnpSsdpResponse {
  // The MediaRenderer's UDN, from the USN, without "uuid:".
  QString id;
  QUrl location;

  // Returns false if |datagram| isn't a MediaRenderer's response.
  bool Parse(const QByteArray& datagram);
};

#endif  // UPNP_UPNPDEVICE_H_
