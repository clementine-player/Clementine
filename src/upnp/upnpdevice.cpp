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

#include "upnpdevice.h"

#include <QDebug>
#include <QList>
#include <QStringList>
#include <QXmlStreamReader>

namespace {

const char kMediaRendererType[] = ":device:MediaRenderer:";

// MIME types devices list, and the planner's name for each.
struct MimeAlias {
  const char* device;
  const char* planner;
};
const MimeAlias kMimeAliases[] = {
    {"audio/mpeg", "audio/mpeg"},
    {"audio/mp3", "audio/mpeg"},
    {"audio/mpeg3", "audio/mpeg"},
    {"audio/x-mpeg", "audio/mpeg"},
    {"audio/mp4", "audio/mp4"},
    {"audio/x-m4a", "audio/mp4"},
    {"audio/m4a", "audio/mp4"},
    {"audio/aac", "audio/aac"},
    {"audio/x-aac", "audio/aac"},
    {"audio/vnd.dlna.adts", "audio/aac"},
    {"audio/flac", "audio/flac"},
    {"audio/x-flac", "audio/flac"},
    {"audio/wav", "audio/wav"},
    {"audio/x-wav", "audio/wav"},
    {"audio/wave", "audio/wav"},
    {"audio/aiff", "audio/aiff"},
    {"audio/x-aiff", "audio/aiff"},
    // Ogg is taken to mean Vorbis, which everything that lists it plays.
    {"audio/ogg", "audio/ogg; codecs=vorbis"},
    {"application/ogg", "audio/ogg; codecs=vorbis"},
    {"audio/opus", "audio/ogg; codecs=opus"},
};

// One <device> in a description.
struct DescribedDevice {
  QString type;
  QString friendly_name;
  QString model;
  QString udn;
  QList<UpnpService> services;

  UpnpService Find(const QString& name) const {
    const QString prefix =
        QString("urn:schemas-upnp-org:service:%1:").arg(name);
    for (const UpnpService& service : services) {
      if (service.type.startsWith(prefix)) return service;
    }
    return UpnpService();
  }
};

QString StripUuid(const QString& udn) {
  return udn.startsWith("uuid:") ? udn.mid(5) : udn;
}

}  // namespace

bool UpnpDevice::ParseDescription(const QByteArray& xml, const QUrl& location) {
  QList<DescribedDevice> open;
  QList<DescribedDevice> devices;
  QString url_base;

  QXmlStreamReader reader(xml);
  while (!reader.atEnd() && reader.readNext() != QXmlStreamReader::Invalid) {
    if (reader.isEndElement() && reader.name() == QLatin1String("device")) {
      if (!open.isEmpty()) devices << open.takeLast();
      continue;
    }
    if (!reader.isStartElement()) continue;

    const QStringView name = reader.name();
    if (name == QLatin1String("device")) {
      open << DescribedDevice();
    } else if (open.isEmpty()) {
      if (name == QLatin1String("URLBase")) url_base = reader.readElementText();
    } else if (name == QLatin1String("deviceType")) {
      open.last().type = reader.readElementText();
    } else if (name == QLatin1String("friendlyName")) {
      open.last().friendly_name = reader.readElementText();
    } else if (name == QLatin1String("modelName")) {
      open.last().model = reader.readElementText();
    } else if (name == QLatin1String("UDN")) {
      open.last().udn = reader.readElementText();
    } else if (name == QLatin1String("service")) {
      UpnpService service;
      QString control_url;
      QString event_url;
      while (reader.readNextStartElement()) {
        if (reader.name() == QLatin1String("serviceType")) {
          service.type = reader.readElementText();
        } else if (reader.name() == QLatin1String("controlURL")) {
          control_url = reader.readElementText();
        } else if (reader.name() == QLatin1String("eventSubURL")) {
          event_url = reader.readElementText().trimmed();
        } else {
          reader.skipCurrentElement();
        }
      }
      // Resolved once URLBase is known, which may come after.
      service.control_url = QUrl(control_url);
      if (!event_url.isEmpty()) service.event_url = QUrl(event_url);
      open.last().services << service;
    }
  }
  // The root closes last.
  if (devices.isEmpty()) return false;
  const DescribedDevice& root = devices.last();

  const DescribedDevice* renderer = nullptr;
  for (const DescribedDevice& device : devices) {
    if (device.type.contains(kMediaRendererType)) {
      renderer = &device;
      break;
    }
  }
  if (!renderer || renderer->udn.isEmpty()) return false;

  const QUrl base = url_base.isEmpty() ? location : QUrl(url_base);
  auto resolved = [&base](UpnpService service) {
    if (service.is_valid()) {
      service.control_url = base.resolved(service.control_url);
      if (!service.event_url.isEmpty()) {
        service.event_url = base.resolved(service.event_url);
      }
    }
    return service;
  };

  const UpnpService new_av_transport = resolved(renderer->Find("AVTransport"));
  if (!new_av_transport.is_valid()) return false;

  id = StripUuid(renderer->udn);
  name = renderer->friendly_name.isEmpty() ? root.friendly_name
                                           : renderer->friendly_name;
  model = renderer->model.isEmpty() ? root.model : renderer->model;
  address = QHostAddress(location.host());
  av_transport = new_av_transport;
  rendering_control = resolved(renderer->Find("RenderingControl"));
  connection_manager = resolved(renderer->Find("ConnectionManager"));
  group_rendering_control = resolved(renderer->Find("GroupRenderingControl"));
  root_id = StripUuid(root.udn);
  zone_group_topology = resolved(root.Find("ZoneGroupTopology"));
  return true;
}

void UpnpDevice::SetProtocolInfo(const QString& sink) {
  caps = RendererCaps();
  device_mime_types.clear();
  video = false;

  for (const QString& entry : sink.split(',')) {
    // "protocol:network:contentFormat:additionalInfo"
    const QStringList parts = entry.trimmed().split(':');
    if (parts.size() != 4 || parts[0] != "http-get") continue;
    const QString device_mime = parts[2].toLower();
    if (device_mime.startsWith("video/")) video = true;

    for (const MimeAlias& alias : kMimeAliases) {
      if (device_mime != alias.device) continue;
      const QString planner_mime = alias.planner;
      if (device_mime_types.contains(planner_mime)) break;
      device_mime_types[planner_mime] = parts[2];

      RendererCaps::Format format;
      format.mime_type = planner_mime;
      caps.formats << format;
      break;
    }
  }
  // Devices that play files over HTTP fetch them with ranges.
  caps.http_range = true;
}

QString UpnpDevice::DeviceMimeType(const QString& mime_type) const {
  const RendererCaps::Format* format = caps.Find(mime_type);
  if (!format) return mime_type;
  return device_mime_types.value(format->mime_type, mime_type);
}

bool UpnpDevice::operator==(const UpnpDevice& other) const {
  return id == other.id && name == other.name && model == other.model &&
         address == other.address && av_transport == other.av_transport &&
         rendering_control == other.rendering_control &&
         connection_manager == other.connection_manager &&
         group_rendering_control == other.group_rendering_control &&
         root_id == other.root_id &&
         zone_group_topology == other.zone_group_topology &&
         device_mime_types == other.device_mime_types && video == other.video &&
         group == other.group;
}

QDebug operator<<(QDebug dbg, const UpnpDevice& device) {
  QDebugStateSaver saver(dbg);
  dbg.nospace().noquote() << device.name << " (" << device.model << ", "
                          << device.id << ") at " << device.address.toString();
  return dbg;
}

bool UpnpSsdpResponse::Parse(const QByteArray& datagram) {
  const QList<QByteArray> lines = datagram.split('\n');
  if (lines.isEmpty() || !lines[0].startsWith("HTTP/1.1 200")) return false;

  QString usn;
  QString st;
  QUrl new_location;
  for (const QByteArray& line : lines) {
    const int colon = line.indexOf(':');
    if (colon <= 0) continue;
    const QByteArray key = line.left(colon).trimmed().toLower();
    const QString value = QString::fromUtf8(line.mid(colon + 1).trimmed());
    if (key == "usn") {
      usn = value;
    } else if (key == "st") {
      st = value;
    } else if (key == "location") {
      new_location = QUrl(value);
    }
  }

  // Some devices answer every search, with whatever they are.
  // "uuid:RINCON_000E5876740201400_MR::urn:schemas-upnp-org:device:..."
  if (!st.contains(kMediaRendererType) || !usn.contains(kMediaRendererType) ||
      !usn.startsWith("uuid:")) {
    return false;
  }
  const QString new_id = usn.mid(5).section("::", 0, 0);
  if (new_id.isEmpty() || !new_location.isValid() ||
      new_location.scheme() != "http") {
    return false;
  }

  id = new_id;
  location = new_location;
  return true;
}
