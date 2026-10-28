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

#include <QImage>
#include <QNetworkAccessManager>
#include <QSignalSpy>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTemporaryDir>
#include <QTest>
#include <memory>

#include "gtest/gtest.h"
#include "networkremote/streaming/eventcallbacktable.h"
#include "networkremote/streaming/mediahttpserver.h"
#include "networkremote/streaming/streamitemtable.h"
#include "test_utils.h"
#include "upnp/sonostopology.h"
#include "upnp/upnpdevice.h"
#include "upnp/upnpdiscovery.h"
#include "upnp/upnpengine.h"
#include "upnp/upnpmediaplayer.h"
#include "upnp/upnpsoap.h"
#include "upnp/upnpsubscription.h"

namespace {

// What a Sonos Play:3 answers, as sent.
const char kTransportInfo[] =
    "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" "
    "s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"><s:Body>"
    "<u:GetTransportInfoResponse "
    "xmlns:u=\"urn:schemas-upnp-org:service:AVTransport:1\">"
    "<CurrentTransportState>STOPPED</CurrentTransportState>"
    "<CurrentTransportStatus>OK</CurrentTransportStatus>"
    "<CurrentSpeed>1</CurrentSpeed></u:GetTransportInfoResponse></s:Body>"
    "</s:Envelope>";

const char kFault[] =
    "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" "
    "s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"><s:Body>"
    "<s:Fault><faultcode>s:Client</faultcode><faultstring>UPnPError"
    "</faultstring><detail><UPnPError "
    "xmlns=\"urn:schemas-upnp-org:control-1-0\"><errorCode>714</errorCode>"
    "</UPnPError></detail></s:Fault></s:Body></s:Envelope>";

const char kSsdpResponse[] =
    "HTTP/1.1 200 OK\r\n"
    "CACHE-CONTROL: max-age = 1800\r\n"
    "EXT:\r\n"
    "LOCATION: http://192.168.86.217:1400/xml/device_description.xml\r\n"
    "SERVER: Linux UPnP/1.0 Sonos/57.22-63071 (ZPS3)\r\n"
    "ST: urn:schemas-upnp-org:device:MediaRenderer:1\r\n"
    "USN: uuid:RINCON_000E5876740201400_MR::urn:schemas-upnp-org:device:"
    "MediaRenderer:1\r\n"
    "X-RINCON-HOUSEHOLD: Sonos_hPhZ76pt6Ycu6KprYWGMoEebdL\r\n"
    "\r\n";

// A Hue bridge answers every search as a root device.
const char kOtherSsdpResponse[] =
    "HTTP/1.1 200 OK\r\n"
    "LOCATION: http://192.168.86.93:80/description.xml\r\n"
    "SERVER: Hue/1.0 UPnP/1.0 IpBridge/1.78.0\r\n"
    "ST: upnp:rootdevice\r\n"
    "USN: uuid:2f402f80-da50-11e1-9b23-ecb5fa956bf0::upnp:rootdevice\r\n"
    "\r\n";

// A Sonos Play:3's description, trimmed: a ZonePlayer with a MediaServer
// and a MediaRenderer in it.
const char kSonosDescription[] =
    "<?xml version=\"1.0\" encoding=\"utf-8\" ?>"
    "<root xmlns=\"urn:schemas-upnp-org:device-1-0\">"
    "<specVersion><major>1</major><minor>0</minor></specVersion>"
    "<device>"
    "<deviceType>urn:schemas-upnp-org:device:ZonePlayer:1</deviceType>"
    "<friendlyName>192.168.86.217 - Sonos Play:3</friendlyName>"
    "<modelName>Sonos Play:3</modelName>"
    "<UDN>uuid:RINCON_000E5876740201400</UDN>"
    "<roomName>Bedroom</roomName>"
    "<serviceList>"
    "<service><serviceType>urn:schemas-upnp-org:service:ZoneGroupTopology:1"
    "</serviceType><serviceId>urn:upnp-org:serviceId:ZoneGroupTopology"
    "</serviceId><controlURL>/ZoneGroupTopology/Control</controlURL>"
    "<eventSubURL>/ZoneGroupTopology/Event</eventSubURL></service>"
    "</serviceList>"
    "<deviceList>"
    "<device>"
    "<deviceType>urn:schemas-upnp-org:device:MediaServer:1</deviceType>"
    "<friendlyName>192.168.86.217 - Sonos Play:3 Media Server</friendlyName>"
    "<UDN>uuid:RINCON_000E5876740201400_MS</UDN>"
    "<serviceList>"
    "<service><serviceType>urn:schemas-upnp-org:service:ConnectionManager:1"
    "</serviceType><controlURL>/MediaServer/ConnectionManager/Control"
    "</controlURL></service>"
    "</serviceList>"
    "</device>"
    "<device>"
    "<deviceType>urn:schemas-upnp-org:device:MediaRenderer:1</deviceType>"
    "<friendlyName>Bedroom - Sonos Play:3 Media Renderer</friendlyName>"
    "<modelName>Sonos Play:3</modelName>"
    "<UDN>uuid:RINCON_000E5876740201400_MR</UDN>"
    "<serviceList>"
    "<service><serviceType>urn:schemas-upnp-org:service:RenderingControl:1"
    "</serviceType><controlURL>/MediaRenderer/RenderingControl/Control"
    "</controlURL></service>"
    "<service><serviceType>urn:schemas-upnp-org:service:ConnectionManager:1"
    "</serviceType><controlURL>/MediaRenderer/ConnectionManager/Control"
    "</controlURL></service>"
    "<service><serviceType>urn:schemas-upnp-org:service:AVTransport:1"
    "</serviceType><controlURL>/MediaRenderer/AVTransport/Control"
    "</controlURL><eventSubURL>/MediaRenderer/AVTransport/Event"
    "</eventSubURL></service>"
    "<service><serviceType>"
    "urn:schemas-upnp-org:service:GroupRenderingControl:1</serviceType>"
    "<controlURL>/MediaRenderer/GroupRenderingControl/Control</controlURL>"
    "</service>"
    "</serviceList>"
    "</device>"
    "</deviceList>"
    "</device>"
    "</root>";

// A renderer that's the root device, with a URLBase and relative control
// URLs.
const char kPlainDescription[] =
    "<?xml version=\"1.0\"?>"
    "<root xmlns=\"urn:schemas-upnp-org:device-1-0\">"
    "<URLBase>http://10.0.0.9:49152/</URLBase>"
    "<device>"
    "<deviceType>urn:schemas-upnp-org:device:MediaRenderer:1</deviceType>"
    "<friendlyName>Kitchen &amp; Dining</friendlyName>"
    "<modelName>Renderer</modelName>"
    "<UDN>uuid:5f9ec1b3-ed59-79bb-4530-745d6b3e9a65</UDN>"
    "<dlna:X_DLNADOC xmlns:dlna=\"urn:schemas-dlna-org:device-1-0\">"
    "DMR-1.50</dlna:X_DLNADOC>"
    "<serviceList>"
    "<service><serviceType>urn:schemas-upnp-org:service:AVTransport:2"
    "</serviceType><controlURL>upnp/control/avt</controlURL>"
    "<eventSubURL>upnp/event/avt</eventSubURL></service>"
    "<service><serviceType>urn:schemas-upnp-org:service:RenderingControl:2"
    "</serviceType><controlURL>upnp/control/rc</controlURL></service>"
    "</serviceList>"
    "</device>"
    "</root>";

// A Sonos player's Sink, trimmed.
const char kSink[] =
    "http-get:*:audio/mp3:*,x-file-cifs:*:audio/mp3:*,http-get:*:audio/mp4:*,"
    "http-get:*:audio/x-m4a:*,http-get:*:audio/mpeg:*,http-get:*:audio/wav:*,"
    "http-get:*:audio/x-ms-wma:*,http-get:*:audio/aiff:*,"
    "http-get:*:audio/flac:*,http-get:*:application/ogg:*,"
    "http-get:*:audio/ogg:*,sonos.com-http:*:audio/aac:*,x-rincon:*:*:*,"
    "x-sonosapi-hls:*:*:*";

// Two rooms on their own, as S1 players have it, trimmed of attributes
// that aren't used.
const char kTwoRooms[] =
    "<ZoneGroupState><ZoneGroups>"
    "<ZoneGroup Coordinator=\"RINCON_000E5876740201400\" "
    "ID=\"RINCON_000E5876740201400:16\">"
    "<ZoneGroupMember UUID=\"RINCON_000E5876740201400\" "
    "Location=\"http://192.168.86.217:1400/xml/device_description.xml\" "
    "ZoneName=\"Bedroom\" SoftwareVersion=\"57.22-63071\" SWGen=\"1\"/>"
    "</ZoneGroup>"
    "<ZoneGroup Coordinator=\"RINCON_000E5882269001400\" "
    "ID=\"RINCON_000E5882269001400:3001943152\">"
    "<ZoneGroupMember UUID=\"RINCON_000E5882269001400\" "
    "Location=\"http://192.168.86.59:1400/xml/device_description.xml\" "
    "ZoneName=\"Lounge\" SoftwareVersion=\"57.23-74170\" SWGen=\"1\"/>"
    "</ZoneGroup>"
    "</ZoneGroups><VanishedDevices></VanishedDevices></ZoneGroupState>";

// The Lounge, a stereo pair with a Sub, grouped with the Kitchen. Older
// firmware has no <ZoneGroupState>.
const char kGrouped[] =
    "<ZoneGroups>"
    "<ZoneGroup Coordinator=\"RINCON_B\" ID=\"RINCON_B:1\">"
    "<ZoneGroupMember UUID=\"RINCON_A\" ZoneName=\"Kitchen\"/>"
    "<ZoneGroupMember UUID=\"RINCON_B\" ZoneName=\"Lounge\">"
    "<Satellite UUID=\"RINCON_SUB\" ZoneName=\"Lounge\"/>"
    "</ZoneGroupMember>"
    "<ZoneGroupMember UUID=\"RINCON_C\" ZoneName=\"Lounge\" Invisible=\"1\"/>"
    "</ZoneGroup>"
    "</ZoneGroups>";

UpnpDevice TestDevice() {
  UpnpDevice device;
  EXPECT_TRUE(device.ParseDescription(
      kSonosDescription,
      QUrl("http://10.0.0.2:1400/xml/device_description.xml")));
  device.SetProtocolInfo(kSink);
  return device;
}

TEST(UpnpSoapTest, Envelope) {
  UpnpSoap::Action action;
  action.service_type = "urn:schemas-upnp-org:service:AVTransport:1";
  action.name = "Seek";
  action.args = {{"InstanceID", "0"}, {"Unit", "REL_TIME"}, {"Target", "a<b"}};
  const QString envelope = QString::fromUtf8(UpnpSoap::Envelope(action));
  EXPECT_TRUE(envelope.contains(
      "<s:Body><u:Seek xmlns:u=\"urn:schemas-upnp-org:service:AVTransport:1\">"
      "<InstanceID>0</InstanceID><Unit>REL_TIME</Unit>"
      "<Target>a&lt;b</Target></u:Seek></s:Body>"))
      << envelope.toStdString();
}

TEST(UpnpSoapTest, ParsesAResponse) {
  const UpnpSoap::Response response = UpnpSoap::Parse(200, kTransportInfo);
  EXPECT_TRUE(response.ok);
  EXPECT_EQ("STOPPED", response.values.value("CurrentTransportState"));
  EXPECT_EQ("OK", response.values.value("CurrentTransportStatus"));
  EXPECT_EQ(3, response.values.size());
}

TEST(UpnpSoapTest, ParsesAFault) {
  const UpnpSoap::Response response = UpnpSoap::Parse(500, kFault);
  EXPECT_FALSE(response.ok);
  EXPECT_EQ(500, response.http_status);
  EXPECT_EQ(714, response.error_code);
  EXPECT_EQ("UPnP error 714", response.error);
}

TEST(UpnpSoapTest, NetworkErrors) {
  const UpnpSoap::Response response =
      UpnpSoap::Parse(0, QByteArray(), "Connection refused");
  EXPECT_FALSE(response.ok);
  EXPECT_EQ(0, response.http_status);
  EXPECT_EQ("Connection refused", response.error);

  EXPECT_FALSE(UpnpSoap::Parse(200, "not xml").ok);
}

TEST(UpnpDeviceTest, ParsesAnSsdpResponse) {
  UpnpSsdpResponse response;
  ASSERT_TRUE(response.Parse(kSsdpResponse));
  EXPECT_EQ("RINCON_000E5876740201400_MR", response.id);
  EXPECT_EQ(QUrl("http://192.168.86.217:1400/xml/device_description.xml"),
            response.location);
}

TEST(UpnpDeviceTest, IgnoresOtherSsdpResponses) {
  UpnpSsdpResponse response;
  EXPECT_FALSE(response.Parse(kOtherSsdpResponse));
  EXPECT_FALSE(response.Parse(QByteArray(kSsdpResponse).replace("200", "404")));
  EXPECT_FALSE(response.Parse(
      QByteArray(kSsdpResponse).replace("LOCATION: http:", "LOCATION: ftp:")));
  EXPECT_FALSE(response.Parse(UpnpDiscovery::SearchMessage()));
}

TEST(UpnpDeviceTest, ParsesAnEmbeddedRenderer) {
  UpnpDevice device;
  ASSERT_TRUE(device.ParseDescription(
      kSonosDescription,
      QUrl("http://192.168.86.217:1400/xml/device_description.xml")));
  EXPECT_TRUE(device.is_valid());
  EXPECT_EQ("RINCON_000E5876740201400_MR", device.id);
  EXPECT_EQ("Bedroom - Sonos Play:3 Media Renderer", device.name);
  EXPECT_EQ("Sonos Play:3", device.model);
  EXPECT_EQ(QHostAddress("192.168.86.217"), device.address);
  EXPECT_EQ(
      QUrl("http://192.168.86.217:1400/MediaRenderer/AVTransport/Control"),
      device.av_transport.control_url);
  EXPECT_EQ("urn:schemas-upnp-org:service:AVTransport:1",
            device.av_transport.type);
  EXPECT_EQ(QUrl("http://192.168.86.217:1400/MediaRenderer/AVTransport/Event"),
            device.av_transport.event_url);
  // The fixture leaves it out.
  EXPECT_TRUE(device.rendering_control.event_url.isEmpty());
  // Sonos doesn't say it follows DLNA.
  EXPECT_FALSE(device.dlna);
  // The renderer's, not the MediaServer's.
  EXPECT_EQ(QUrl("http://192.168.86.217:1400/MediaRenderer/ConnectionManager/"
                 "Control"),
            device.connection_manager.control_url);
  EXPECT_TRUE(device.rendering_control.is_valid());
  EXPECT_TRUE(device.group_rendering_control.is_valid());
  // The root's.
  EXPECT_EQ("RINCON_000E5876740201400", device.root_id);
  EXPECT_EQ(QUrl("http://192.168.86.217:1400/ZoneGroupTopology/Control"),
            device.zone_group_topology.control_url);
}

TEST(UpnpDeviceTest, ParsesARootRenderer) {
  UpnpDevice device;
  ASSERT_TRUE(device.ParseDescription(
      kPlainDescription, QUrl("http://10.0.0.9:49152/description.xml")));
  EXPECT_EQ("5f9ec1b3-ed59-79bb-4530-745d6b3e9a65", device.id);
  EXPECT_EQ(device.id, device.root_id);
  EXPECT_EQ("Kitchen & Dining", device.name);
  EXPECT_EQ(QUrl("http://10.0.0.9:49152/upnp/control/avt"),
            device.av_transport.control_url);
  EXPECT_EQ(QUrl("http://10.0.0.9:49152/upnp/event/avt"),
            device.av_transport.event_url);
  EXPECT_TRUE(device.dlna);
  EXPECT_EQ("urn:schemas-upnp-org:service:AVTransport:2",
            device.av_transport.type);
  EXPECT_FALSE(device.connection_manager.is_valid());
  EXPECT_FALSE(device.group_rendering_control.is_valid());
  EXPECT_FALSE(device.zone_group_topology.is_valid());
}

TEST(UpnpDeviceTest, NeedsAnAVTransport) {
  UpnpDevice device;
  EXPECT_FALSE(device.ParseDescription(
      QByteArray(kPlainDescription).replace("AVTransport", "Something"),
      QUrl("http://10.0.0.9:49152/description.xml")));
  EXPECT_FALSE(device.ParseDescription(
      QByteArray(kPlainDescription).replace("MediaRenderer", "MediaServer"),
      QUrl("http://10.0.0.9:49152/description.xml")));
  EXPECT_FALSE(device.ParseDescription("<html>", QUrl("http://10.0.0.9/")));
}

TEST(UpnpDeviceTest, ProtocolInfo) {
  UpnpDevice device;
  device.SetProtocolInfo(kSink);
  // Each once, over HTTP only, in the planner's names.
  EXPECT_EQ(QStringList() << "audio/mpeg"
                          << "audio/mp4"
                          << "audio/wav"
                          << "audio/aiff"
                          << "audio/flac"
                          << "audio/ogg; codecs=vorbis",
            device.caps.mime_types());
  EXPECT_TRUE(device.caps.http_range);
  // Told it's sending what the device listed first.
  EXPECT_EQ("audio/mp3", device.DeviceMimeType("audio/mpeg"));
  EXPECT_EQ("application/ogg",
            device.DeviceMimeType("audio/ogg; codecs=vorbis"));
  EXPECT_EQ("audio/flac", device.DeviceMimeType("audio/flac"));
  EXPECT_FALSE(device.caps.Accepts("audio/aac"));
  EXPECT_FALSE(device.caps.Accepts("audio/ogg; codecs=opus"));

  // Sonos plays only audio.
  EXPECT_FALSE(device.video);

  device.SetProtocolInfo("");
  EXPECT_TRUE(device.caps.formats.isEmpty());
}

TEST(UpnpDeviceTest, ProtocolInfoWithVideo) {
  // Like a TV's.
  UpnpDevice device;
  device.SetProtocolInfo(
      "http-get:*:audio/mpeg:DLNA.ORG_PN=MP3,"
      "http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_MP_HD_720p_AAC");
  EXPECT_TRUE(device.video);
  EXPECT_EQ(QStringList() << "audio/mpeg", device.caps.mime_types());

  // Not over HTTP.
  device.SetProtocolInfo("rtsp-rtp-udp:*:video/mp4:*,http-get:*:audio/mpeg:*");
  EXPECT_FALSE(device.video);
}

TEST(SonosTopologyTest, RoomsOnTheirOwn) {
  const QList<SonosZoneGroup> groups = SonosZoneGroup::ParseState(kTwoRooms);
  ASSERT_EQ(2, groups.size());
  EXPECT_EQ("RINCON_000E5876740201400", groups[0].coordinator_id);
  ASSERT_EQ(1, groups[0].members.size());
  EXPECT_EQ("RINCON_000E5876740201400", groups[0].members[0].id);
  EXPECT_EQ("Bedroom", groups[0].Name());
  EXPECT_EQ(QStringList() << "Bedroom", groups[0].Rooms());
  EXPECT_EQ("Lounge", groups[1].Name());
}

TEST(SonosTopologyTest, GroupedRooms) {
  const QList<SonosZoneGroup> groups = SonosZoneGroup::ParseState(kGrouped);
  ASSERT_EQ(1, groups.size());
  // The Sub plays with its room, so it isn't a member.
  EXPECT_EQ(3, groups[0].members.size());
  EXPECT_TRUE(groups[0].members[2].invisible);
  // The coordinator first, and the pair once.
  EXPECT_EQ(QStringList() << "Lounge"
                          << "Kitchen",
            groups[0].Rooms());
  EXPECT_EQ("Lounge + Kitchen", groups[0].Name());
}

TEST(UpnpMediaPlayerTest, Times) {
  EXPECT_EQ("0:00:00", UpnpMediaPlayer::FormatTime(0));
  EXPECT_EQ("0:01:05", UpnpMediaPlayer::FormatTime(65999));
  EXPECT_EQ("1:02:03", UpnpMediaPlayer::FormatTime(3723000));

  EXPECT_EQ(65000, UpnpMediaPlayer::ParseTime("0:01:05"));
  EXPECT_EQ(3723500, UpnpMediaPlayer::ParseTime("1:02:03.500"));
  EXPECT_EQ(-1, UpnpMediaPlayer::ParseTime("NOT_IMPLEMENTED"));
  EXPECT_EQ(-1, UpnpMediaPlayer::ParseTime(""));
}

TEST(UpnpMediaPlayerTest, Didl) {
  UpnpMediaInfo info;
  info.url = QUrl("http://10.0.0.1:5500/s/token/1?t=5");
  info.content_type = "audio/ogg; codecs=vorbis";
  info.title = "Fish & Chips";
  info.artist = "<Artist>";
  info.duration_ms = 245000;

  const QString didl = UpnpMediaPlayer::Didl(info);
  EXPECT_TRUE(didl.contains("<dc:title>Fish &amp; Chips</dc:title>"))
      << didl.toStdString();
  EXPECT_TRUE(didl.contains("<dc:creator>&lt;Artist&gt;</dc:creator>"));
  EXPECT_FALSE(didl.contains("upnp:album>"));
  EXPECT_FALSE(didl.contains("albumArtURI"));
  EXPECT_TRUE(didl.contains(
      "<res protocolInfo=\"http-get:*:audio/ogg:*\" duration=\"0:04:05\">"
      "http://10.0.0.1:5500/s/token/1?t=5</res>"))
      << didl.toStdString();
  EXPECT_TRUE(didl.contains("object.item.audioItem.musicTrack"));

  // For a DLNA renderer.
  info.dlna_features = "DLNA.ORG_OP=01;DLNA.ORG_CI=0";
  EXPECT_TRUE(UpnpMediaPlayer::Didl(info).contains(
      "protocolInfo=\"http-get:*:audio/ogg:DLNA.ORG_OP=01;DLNA.ORG_CI=0\""));
  info.dlna_features.clear();

  info.image_url = QUrl("http://10.0.0.1:5500/a/token/1");
  EXPECT_TRUE(UpnpMediaPlayer::Didl(info).contains(
      "<upnp:albumArtURI>http://10.0.0.1:5500/a/token/1</upnp:albumArtURI>"));
}

TEST(UpnpEngineTest, EncodesCovers) {
  EXPECT_TRUE(UpnpEngine::EncodeCover(QImage()).isEmpty());

  QImage big(1200, 900, QImage::Format_ARGB32);
  big.fill(Qt::red);
  const QByteArray jpeg = UpnpEngine::EncodeCover(big);
  ASSERT_FALSE(jpeg.isEmpty());
  EXPECT_TRUE(jpeg.startsWith("\xff\xd8"));
  // Scaled down to what renderers show, keeping its shape.
  const QImage decoded = QImage::fromData(jpeg, "JPG");
  EXPECT_EQ(600, decoded.width());
  EXPECT_EQ(450, decoded.height());

  QImage small(200, 200, QImage::Format_RGB32);
  small.fill(Qt::blue);
  EXPECT_EQ(200, QImage::fromData(UpnpEngine::EncodeCover(small)).width());
}

TEST(UpnpEngineTest, LoadsCovers) {
  QImage image(300, 300, QImage::Format_RGB32);
  image.fill(Qt::green);
  QTemporaryDir dir;
  const QString path = dir.filePath("cover.png");
  ASSERT_TRUE(image.save(path));

  Song song;
  song.set_art_automatic(path);
  EXPECT_EQ(300, QImage::fromData(UpnpEngine::LoadCover(song)).width());

  // One the user unset, or on the web, isn't loaded.
  song.set_art_manual(Song::kManuallyUnsetCover);
  EXPECT_TRUE(UpnpEngine::LoadCover(song).isEmpty());
  song.set_art_manual("https://example.com/cover.jpg");
  EXPECT_EQ(300, QImage::fromData(UpnpEngine::LoadCover(song)).width());
  song.set_art_automatic("https://example.com/cover.jpg");
  EXPECT_TRUE(UpnpEngine::LoadCover(song).isEmpty());
}

// Records what's sent, and answers it as the test says.
class FakePlayer : public UpnpMediaPlayer {
 public:
  FakePlayer() { SetDevice(TestDevice()); }

  void Send(const UpnpSoap::Action& action) override { sent_ << action; }

  using UpnpMediaPlayer::TransportEvent;
  using UpnpMediaPlayer::VolumeEvent;

  // Answers the action that was sent last.
  void Answer(const QMap<QString, QString>& values = {}) {
    UpnpSoap::Response response;
    response.ok = true;
    response.http_status = 200;
    response.values = values;
    Finished(response);
  }
  void Fault(int code) {
    UpnpSoap::Response response;
    response.http_status = 500;
    response.error_code = code;
    response.error = QString("UPnP error %1").arg(code);
    Finished(response);
  }
  void Unreachable() {
    UpnpSoap::Response response;
    response.error = "Connection refused";
    Finished(response);
  }

  // A poll, answered with |position| (RelTime) and |state|.
  void Poll(const QString& position, const QString& state,
            const QString& track_uri = QString()) {
    QMetaObject::invokeMethod(this, "Poll", Qt::DirectConnection);
    EXPECT_EQ("GetPositionInfo", last().name.toStdString());
    Answer({{"RelTime", position},
            {"TrackDuration", "0:04:00"},
            {"TrackURI", track_uri.isEmpty() ? kUrl : track_uri}});
    EXPECT_EQ("GetTransportInfo", last().name.toStdString());
    Answer(
        {{"CurrentTransportState", state}, {"CurrentTransportStatus", "OK"}});
  }

  const UpnpSoap::Action& last() const { return sent_.last(); }
  QString Arg(const QString& name) const {
    for (const auto& arg : last().args) {
      if (arg.first == name) return arg.second;
    }
    return QString();
  }

  static const char kUrl[];
  QList<UpnpSoap::Action> sent_;
};

const char FakePlayer::kUrl[] = "http://10.0.0.1:5500/s/token/1";

UpnpMediaInfo Info() {
  UpnpMediaInfo info;
  info.url = QUrl(FakePlayer::kUrl);
  info.content_type = "audio/mpeg";
  info.title = "Title";
  info.duration_ms = 240000;
  return info;
}

// Loads and plays from the start, until the player says it's playing.
void StartPlaying(FakePlayer* player) {
  player->Load(Info(), 0, true);
  player->Answer();  // SetAVTransportURI
  player->Answer();  // Play
  player->Poll("0:00:01", "PLAYING");
}

TEST(UpnpMediaPlayerTest, Loads) {
  FakePlayer player;
  QSignalSpy states(&player, &UpnpMediaPlayer::StateChanged);
  player.Load(Info(), 0, true);

  ASSERT_EQ(1, player.sent_.size());
  EXPECT_EQ("SetAVTransportURI", player.last().name.toStdString());
  EXPECT_EQ(QUrl("http://10.0.0.2:1400/MediaRenderer/AVTransport/Control"),
            player.last().control_url);
  EXPECT_EQ("urn:schemas-upnp-org:service:AVTransport:1",
            player.last().service_type.toStdString());
  EXPECT_EQ("0", player.Arg("InstanceID"));
  EXPECT_EQ(FakePlayer::kUrl, player.Arg("CurrentURI"));
  EXPECT_TRUE(player.Arg("CurrentURIMetaData").contains("<dc:title>Title"));
  EXPECT_EQ(UpnpMediaPlayer::Buffering, player.state());

  // One at a time.
  player.Answer();
  ASSERT_EQ(2, player.sent_.size());
  EXPECT_EQ("Play", player.last().name.toStdString());
  player.Answer();

  player.Poll("0:00:01", "PLAYING");
  EXPECT_EQ(UpnpMediaPlayer::Playing, player.state());
  EXPECT_GE(player.position_ms(), 1000);
  EXPECT_EQ(240000, player.duration_ms());
}

TEST(UpnpMediaPlayerTest, LoadsPartWayThrough) {
  FakePlayer player;
  player.Load(Info(), 65000, true);
  player.Answer();
  EXPECT_EQ("Seek", player.last().name.toStdString());
  EXPECT_EQ("REL_TIME", player.Arg("Unit"));
  EXPECT_EQ("0:01:05", player.Arg("Target"));
  player.Answer();
  EXPECT_EQ("Play", player.last().name.toStdString());
  player.Answer();

  // It isn't sent again.
  const int sent = player.sent_.size();
  player.Poll("0:01:05", "PLAYING");
  EXPECT_EQ(sent + 2, player.sent_.size());
}

TEST(UpnpMediaPlayerTest, SeeksOnceItCan) {
  // Like Rygel, which can't seek until it's playing or paused.
  FakePlayer player;
  player.Load(Info(), 65000, true);
  player.Answer();
  EXPECT_EQ("Seek", player.last().name.toStdString());
  // That doesn't stop it playing.
  player.Fault(710);
  EXPECT_EQ("Play", player.last().name.toStdString());
  player.Answer();
  EXPECT_TRUE(player.is_loaded());

  player.Poll("0:00:00", "TRANSITIONING");
  EXPECT_EQ("GetTransportInfo", player.last().name.toStdString());
  EXPECT_GE(player.position_ms(), 65000);

  player.Poll("0:00:00", "PLAYING");
  EXPECT_EQ("Seek", player.last().name.toStdString());
  EXPECT_EQ("0:01:05", player.Arg("Target"));
  EXPECT_GE(player.position_ms(), 65000);
  player.Answer();

  // Only the once.
  player.Poll("0:01:06", "PLAYING");
  EXPECT_EQ("GetTransportInfo", player.last().name.toStdString());
}

TEST(UpnpMediaPlayerTest, SeekingBeforeItCanMovesTheSeek) {
  FakePlayer player;
  player.Load(Info(), 65000, false);
  player.Answer();
  player.Fault(710);
  EXPECT_TRUE(player.is_loaded());

  // Not sent while it's still stopped.
  const int sent = player.sent_.size();
  player.Seek(30000);
  EXPECT_EQ(sent, player.sent_.size());

  player.Play();
  player.Answer();
  player.Poll("0:00:00", "PLAYING");
  EXPECT_EQ("Seek", player.last().name.toStdString());
  EXPECT_EQ("0:00:30", player.Arg("Target"));
}

TEST(UpnpMediaPlayerTest, LoadsPaused) {
  FakePlayer player;
  player.Load(Info(), 0, false);
  player.Answer();
  EXPECT_EQ(1, player.sent_.size());

  // Stopped is how it is until it's told to play.
  QSignalSpy errors(&player, &UpnpMediaPlayer::Error);
  player.Poll("0:00:00", "STOPPED");
  player.Poll("0:00:00", "STOPPED");
  EXPECT_EQ(0, errors.size());
  EXPECT_TRUE(player.is_loaded());

  player.Play();
  EXPECT_EQ("Play", player.last().name.toStdString());
}

TEST(UpnpMediaPlayerTest, EndsAtTheEnd) {
  FakePlayer player;
  QSignalSpy ended(&player, &UpnpMediaPlayer::TrackEnded);
  QSignalSpy lost(&player, &UpnpMediaPlayer::Lost);
  StartPlaying(&player);

  player.Poll("0:03:58", "PLAYING");
  EXPECT_EQ(0, ended.size());
  // Players go back to the start when they stop.
  player.Poll("0:00:00", "STOPPED");
  EXPECT_EQ(1, ended.size());
  EXPECT_EQ(0, lost.size());
  EXPECT_FALSE(player.is_loaded());
  EXPECT_EQ(UpnpMediaPlayer::Idle, player.state());
}

TEST(UpnpMediaPlayerTest, QueuesTheNext) {
  FakePlayer player;
  QSignalSpy ended(&player, &UpnpMediaPlayer::TrackEnded);
  QSignalSpy next(&player, &UpnpMediaPlayer::NextStarted);
  QSignalSpy lost(&player, &UpnpMediaPlayer::Lost);
  StartPlaying(&player);

  UpnpMediaInfo info = Info();
  info.url = QUrl("http://10.0.0.1:5500/s/token/2");
  info.title = "Next";
  player.SetNext(info);
  EXPECT_EQ("SetNextAVTransportURI", player.last().name.toStdString());
  EXPECT_EQ("http://10.0.0.1:5500/s/token/2", player.Arg("NextURI"));
  EXPECT_TRUE(player.Arg("NextURIMetaData").contains("<dc:title>Next"));
  player.Answer();

  player.Poll("0:03:59", "PLAYING");
  EXPECT_EQ(0, next.size());
  // It's gone on to the next by itself.
  player.Poll("0:00:01", "PLAYING", "http://10.0.0.1:5500/s/token/2");
  EXPECT_EQ(1, next.size());
  EXPECT_EQ(0, ended.size());
  EXPECT_EQ(0, lost.size());
  EXPECT_TRUE(player.is_loaded());
  EXPECT_EQ(UpnpMediaPlayer::Playing, player.state());
  EXPECT_GE(player.position_ms(), 1000);
  EXPECT_LT(player.position_ms(), 3000);

  // Which is what's loaded now.
  player.Poll("0:00:02", "PLAYING", "http://10.0.0.1:5500/s/token/2");
  EXPECT_EQ(1, next.size());
  EXPECT_EQ(0, lost.size());
  player.Poll("0:00:00", "STOPPED", "http://10.0.0.1:5500/s/token/2");
  EXPECT_EQ(0, ended.size());
  EXPECT_EQ(1, lost.size());
}

TEST(UpnpMediaPlayerTest, SomethingElseAfterQueueingTheNext) {
  FakePlayer player;
  QSignalSpy next(&player, &UpnpMediaPlayer::NextStarted);
  QSignalSpy lost(&player, &UpnpMediaPlayer::Lost);
  StartPlaying(&player);
  UpnpMediaInfo info = Info();
  info.url = QUrl("http://10.0.0.1:5500/s/token/2");
  player.SetNext(info);
  player.Answer();

  QMetaObject::invokeMethod(&player, "Poll", Qt::DirectConnection);
  player.Answer({{"RelTime", "0:00:01"},
                 {"TrackURI", "x-rincon-mp3radio://example.com/"}});
  EXPECT_EQ(0, next.size());
  EXPECT_EQ(1, lost.size());
}

TEST(UpnpMediaPlayerTest, CantQueueTheNext) {
  FakePlayer player;
  QSignalSpy ended(&player, &UpnpMediaPlayer::TrackEnded);
  StartPlaying(&player);
  EXPECT_TRUE(player.can_queue_next());

  UpnpMediaInfo info = Info();
  info.url = QUrl("http://10.0.0.1:5500/s/token/2");
  player.SetNext(info);
  // Optional Action Not Implemented.
  player.Fault(602);
  EXPECT_FALSE(player.can_queue_next());
  const int sent = player.sent_.size();
  player.SetNext(info);
  EXPECT_EQ(sent, player.sent_.size());

  // So it ends as it would have.
  player.Poll("0:03:59", "PLAYING");
  player.Poll("0:00:00", "STOPPED");
  EXPECT_EQ(1, ended.size());
}

TEST(UpnpMediaPlayerTest, NextItCouldntQueue) {
  FakePlayer player;
  QSignalSpy ended(&player, &UpnpMediaPlayer::TrackEnded);
  QSignalSpy lost(&player, &UpnpMediaPlayer::Lost);
  StartPlaying(&player);

  UpnpMediaInfo info = Info();
  info.url = QUrl("http://10.0.0.1:5500/s/token/2");
  player.SetNext(info);
  // Illegal MIME-type: only this track's to blame.
  player.Fault(714);
  EXPECT_TRUE(player.can_queue_next());

  player.Poll("0:03:59", "PLAYING");
  player.Poll("0:00:00", "STOPPED");
  EXPECT_EQ(1, ended.size());
  EXPECT_EQ(0, lost.size());
}

TEST(UpnpMediaPlayerTest, LoadingForgetsTheNext) {
  FakePlayer player;
  QSignalSpy next(&player, &UpnpMediaPlayer::NextStarted);
  QSignalSpy lost(&player, &UpnpMediaPlayer::Lost);
  StartPlaying(&player);
  UpnpMediaInfo info = Info();
  info.url = QUrl("http://10.0.0.1:5500/s/token/2");
  player.SetNext(info);
  player.Answer();

  // Like a seek in a stream that can't, which loads it again.
  player.Load(Info(), 30000, true);
  player.Answer();  // SetAVTransportURI
  player.Answer();  // Seek
  player.Answer();  // Play
  player.Poll("0:00:31", "PLAYING");
  QMetaObject::invokeMethod(&player, "Poll", Qt::DirectConnection);
  player.Answer({{"RelTime", "0:00:01"},
                 {"TrackURI", "http://10.0.0.1:5500/s/token/2"}});
  EXPECT_EQ(0, next.size());
  EXPECT_EQ(1, lost.size());
}

TEST(UpnpMediaPlayerTest, StoppedElsewhere) {
  FakePlayer player;
  QSignalSpy ended(&player, &UpnpMediaPlayer::TrackEnded);
  QSignalSpy lost(&player, &UpnpMediaPlayer::Lost);
  StartPlaying(&player);

  player.Poll("0:01:00", "PLAYING");
  player.Poll("0:00:00", "STOPPED");
  EXPECT_EQ(0, ended.size());
  EXPECT_EQ(1, lost.size());
  EXPECT_FALSE(player.is_loaded());
}

TEST(UpnpMediaPlayerTest, SomethingElsePlaying) {
  FakePlayer player;
  QSignalSpy lost(&player, &UpnpMediaPlayer::Lost);
  StartPlaying(&player);

  QMetaObject::invokeMethod(&player, "Poll", Qt::DirectConnection);
  const int sent = player.sent_.size();
  player.Answer({{"RelTime", "0:00:10"},
                 {"TrackURI", "x-sonos-spotify:spotify%3atrack%3a1"}});
  EXPECT_EQ(1, lost.size());
  EXPECT_FALSE(player.is_loaded());
  // Nothing more is asked about it, including the GetTransportInfo that was
  // waiting.
  QMetaObject::invokeMethod(&player, "Poll", Qt::DirectConnection);
  EXPECT_EQ(sent, player.sent_.size());
}

TEST(UpnpMediaPlayerTest, PausedElsewhere) {
  FakePlayer player;
  StartPlaying(&player);
  player.Poll("0:00:10", "PAUSED_PLAYBACK");
  EXPECT_EQ(UpnpMediaPlayer::Paused, player.state());
  EXPECT_TRUE(player.is_loaded());
}

TEST(UpnpMediaPlayerTest, CantPlayTheUri) {
  FakePlayer player;
  QSignalSpy errors(&player, &UpnpMediaPlayer::Error);
  player.Load(Info(), 0, true);
  // 714: illegal MIME type.
  player.Fault(714);
  EXPECT_EQ(1, errors.size());
  // Not told to play it anyway.
  EXPECT_EQ(1, player.sent_.size());
  EXPECT_FALSE(player.is_loaded());
}

TEST(UpnpMediaPlayerTest, NeverStarts) {
  FakePlayer player;
  QSignalSpy errors(&player, &UpnpMediaPlayer::Error);
  player.Load(Info(), 0, true);
  player.Answer();
  player.Answer();

  player.Poll("0:00:00", "TRANSITIONING");
  player.Poll("0:00:00", "STOPPED");
  EXPECT_EQ(0, errors.size());
  player.Poll("0:00:00", "STOPPED");
  EXPECT_EQ(1, errors.size());
  EXPECT_FALSE(player.is_loaded());
}

TEST(UpnpMediaPlayerTest, Unreachable) {
  FakePlayer player;
  QSignalSpy lost(&player, &UpnpMediaPlayer::Lost);
  StartPlaying(&player);

  QMetaObject::invokeMethod(&player, "Poll", Qt::DirectConnection);
  player.Unreachable();
  ASSERT_EQ(1, lost.size());
  EXPECT_EQ("Connection refused", lost[0][0].toString());
  EXPECT_FALSE(player.is_loaded());
}

TEST(UpnpMediaPlayerTest, OldAnswersAreIgnored) {
  FakePlayer player;
  QSignalSpy errors(&player, &UpnpMediaPlayer::Error);
  player.Load(Info(), 0, true);
  // Another track before the first's SetAVTransportURI is answered.
  player.Load(Info(), 0, true);
  player.Fault(714);
  EXPECT_EQ(0, errors.size());
  EXPECT_TRUE(player.is_loaded());
  EXPECT_EQ("SetAVTransportURI", player.last().name.toStdString());
}

TEST(UpnpMediaPlayerTest, Stops) {
  FakePlayer player;
  StartPlaying(&player);
  player.Stop();
  EXPECT_EQ("Stop", player.last().name.toStdString());
  EXPECT_FALSE(player.is_loaded());
  EXPECT_EQ(UpnpMediaPlayer::Idle, player.state());
}

TEST(UpnpMediaPlayerTest, Volume) {
  FakePlayer player;
  player.Load(Info(), 0, false);
  // Waiting behind the load, only the last one is sent.
  player.SetVolume(10);
  player.SetVolume(20);
  player.SetVolume(130);
  player.Answer();
  EXPECT_EQ("SetGroupVolume", player.last().name.toStdString());
  EXPECT_EQ(QUrl("http://10.0.0.2:1400/MediaRenderer/GroupRenderingControl/"
                 "Control"),
            player.last().control_url);
  EXPECT_EQ("100", player.Arg("DesiredVolume"));
  player.Answer();
  EXPECT_EQ(2, player.sent_.size());
}

TEST(UpnpMediaPlayerTest, VolumeWithoutGroups) {
  FakePlayer player;
  UpnpDevice device = TestDevice();
  device.group_rendering_control = UpnpService();
  player.SetDevice(device);
  player.SetVolume(42);
  EXPECT_EQ("SetVolume", player.last().name.toStdString());
  EXPECT_EQ("0", player.Arg("InstanceID"));
  EXPECT_EQ("Master", player.Arg("Channel"));
  EXPECT_EQ("42", player.Arg("DesiredVolume"));
}

// Events from a Sonos Play:3, as they first come after subscribing. The
// AVTransport's is trimmed of variables that aren't used.
const char kGroupVolumeEvent[] =
    "<e:propertyset xmlns:e=\"urn:schemas-upnp-org:event-1-0\">"
    "<e:property><GroupVolume>20</GroupVolume></e:property>"
    "<e:property><GroupMute>0</GroupMute></e:property>"
    "<e:property><GroupVolumeChangeable>1</GroupVolumeChangeable>"
    "</e:property></e:propertyset>";
const char kRenderingControlEvent[] =
    "<e:propertyset xmlns:e=\"urn:schemas-upnp-org:event-1-0\"><e:property>"
    "<LastChange>&lt;Event xmlns=&quot;urn:schemas-upnp-org:metadata-1-0/RCS/"
    "&quot;&gt;&lt;InstanceID val=&quot;0&quot;&gt;&lt;Volume channel=&quot;"
    "Master&quot; val=&quot;20&quot;/&gt;&lt;Volume channel=&quot;LF&quot; "
    "val=&quot;100&quot;/&gt;&lt;Volume channel=&quot;RF&quot; val=&quot;100"
    "&quot;/&gt;&lt;Mute channel=&quot;Master&quot; val=&quot;0&quot;/&gt;"
    "&lt;Mute channel=&quot;LF&quot; val=&quot;0&quot;/&gt;&lt;Bass val=&quot;"
    "0&quot;/&gt;&lt;/InstanceID&gt;&lt;/Event&gt;</LastChange></e:property>"
    "</e:propertyset>";
const char kTransportEvent[] =
    "<e:propertyset xmlns:e=\"urn:schemas-upnp-org:event-1-0\"><e:property>"
    "<LastChange>&lt;Event xmlns=&quot;urn:schemas-upnp-org:metadata-1-0/AVT/"
    "&quot; xmlns:r=&quot;urn:schemas-rinconnetworks-com:metadata-1-0/&quot;"
    "&gt;&lt;InstanceID val=&quot;0&quot;&gt;&lt;TransportState val=&quot;"
    "STOPPED&quot;/&gt;&lt;CurrentTrackURI val=&quot;http://192.168.86.214:"
    "34637/s/c18b74a22c9b820333ab8cce5749eb6b/2&quot;/&gt;&lt;"
    "CurrentTrackDuration val=&quot;0:00:12&quot;/&gt;&lt;CurrentTrackMetaData "
    "val=&quot;&amp;lt;DIDL-Lite&amp;gt;&amp;lt;/DIDL-Lite&amp;gt;&quot;/&gt;"
    "&lt;r:NextTrackURI val=&quot;&quot;/&gt;&lt;TransportStatus val=&quot;OK"
    "&quot;/&gt;&lt;/InstanceID&gt;&lt;/Event&gt;</LastChange></e:property>"
    "</e:propertyset>";

TEST(UpnpSubscriptionTest, ParsesProperties) {
  const QMap<QString, QString> group =
      UpnpSubscription::ParsePropertySet(kGroupVolumeEvent);
  EXPECT_EQ("20", group.value("GroupVolume"));
  EXPECT_EQ("0", group.value("GroupMute"));
  EXPECT_EQ(3, group.size());
}

TEST(UpnpSubscriptionTest, ParsesLastChange) {
  const QMap<QString, QString> volume =
      UpnpSubscription::ParsePropertySet(kRenderingControlEvent);
  // The Master channel's, not the others'.
  EXPECT_EQ("20", volume.value("Volume"));
  EXPECT_EQ("0", volume.value("Mute"));
  EXPECT_EQ("0", volume.value("Bass"));
  EXPECT_FALSE(volume.contains("LastChange"));

  const QMap<QString, QString> transport =
      UpnpSubscription::ParsePropertySet(kTransportEvent);
  EXPECT_EQ("STOPPED", transport.value("TransportState"));
  EXPECT_EQ("OK", transport.value("TransportStatus"));
  EXPECT_EQ("http://192.168.86.214:34637/s/c18b74a22c9b820333ab8cce5749eb6b/2",
            transport.value("CurrentTrackURI"));
  EXPECT_EQ("<DIDL-Lite></DIDL-Lite>",
            transport.value("CurrentTrackMetaData"));
  // Sonos's own, with its prefix.
  EXPECT_TRUE(transport.contains("r:NextTrackURI"));
}

TEST(UpnpSubscriptionTest, OnlyInstanceZero) {
  const QMap<QString, QString> variables = UpnpSubscription::ParseLastChange(
      "<Event><InstanceID val=\"1\"><TransportState val=\"PLAYING\"/>"
      "</InstanceID><InstanceID val=\"0\"><TransportState val=\"PAUSED_"
      "PLAYBACK\"/></InstanceID></Event>");
  EXPECT_EQ("PAUSED_PLAYBACK", variables.value("TransportState"));
  EXPECT_TRUE(UpnpSubscription::ParseLastChange("<html>").isEmpty());
}

// Sends |chunks| to |server| one after another, as a renderer at 127.0.0.1
// would, and returns the answer.
QByteArray Exchange(MediaHttpServer* server, const QList<QByteArray>& chunks) {
  QTcpServer listener;
  if (!listener.listen(QHostAddress::LocalHost)) return QByteArray();
  QObject::connect(&listener, &QTcpServer::newConnection, [&]() {
    while (listener.hasPendingConnections()) {
      server->HandleConnection(listener.nextPendingConnection());
    }
  });

  QTcpSocket socket;
  QByteArray response;
  QObject::connect(&socket, &QTcpSocket::readyRead,
                   [&]() { response += socket.readAll(); });
  socket.connectToHost(QHostAddress::LocalHost, listener.serverPort());
  if (!QTest::qWaitFor(
          [&]() { return socket.state() == QAbstractSocket::ConnectedState; },
          5000)) {
    return QByteArray();
  }
  for (const QByteArray& chunk : chunks) {
    socket.write(chunk);
    socket.flush();
    QTest::qWait(50);
  }
  QTest::qWaitFor(
      [&]() { return socket.state() == QAbstractSocket::UnconnectedState; },
      5000);
  return response + socket.readAll();
}

QByteArray Notify(const QByteArray& path, const QByteArray& body) {
  return "NOTIFY " + path +
         " HTTP/1.1\r\nHOST: x\r\nNT: upnp:event\r\nNTS: upnp:propchange\r\n"
         "SID: uuid:1\r\nSEQ: 0\r\nContent-Length: " +
         QByteArray::number(body.size()) + "\r\n\r\n";
}

TEST(UpnpSubscriptionTest, MediaServerHandsEventsOn) {
  auto items = std::make_shared<StreamItemTable>();
  auto callbacks = std::make_shared<EventCallbackTable>();
  MediaHttpServer server(items, callbacks);
  UpnpSubscription subscription(nullptr, callbacks, QUrl("http://x/event"),
                                QHostAddress::LocalHost);
  callbacks->Set("token", QHostAddress::LocalHost, &subscription);
  QSignalSpy events(&subscription, &UpnpSubscription::Event);

  // The body after its head, as it may come.
  const QByteArray body(kGroupVolumeEvent);
  const QByteArray reply = Exchange(&server, {Notify("/e/token", body), body});
  EXPECT_TRUE(reply.startsWith("HTTP/1.1 200 OK\r\n")) << reply.data();
  // Delivered while that ran, or soon after.
  ASSERT_TRUE(QTest::qWaitFor([&]() { return events.size() == 1; }, 2000));
  const auto variables = events[0][0].value<QMap<QString, QString>>();
  EXPECT_EQ("20", variables.value("GroupVolume"));

  // Nobody else's.
  EXPECT_TRUE(Exchange(&server, {Notify("/e/other", body) + body})
                  .startsWith("HTTP/1.1 404"));
  EXPECT_TRUE(
      Exchange(&server, {"NOTIFY /e/token HTTP/1.1\r\nHOST: x\r\n\r\n"})
          .startsWith("HTTP/1.1 411"));
  callbacks->Set("token", QHostAddress("10.0.0.9"), &subscription);
  EXPECT_TRUE(Exchange(&server, {Notify("/e/token", body) + body})
                  .startsWith("HTTP/1.1 403"));
  // Not without a table.
  MediaHttpServer without(items);
  EXPECT_TRUE(Exchange(&without, {Notify("/e/token", body) + body})
                  .startsWith("HTTP/1.1 404"));
  EXPECT_EQ(1, events.size());
}

// A renderer's event URL, answering SUBSCRIBE and UNSUBSCRIBE as |answer|
// says, one request per connection, and keeping the requests' heads.
class FakeEventServer {
 public:
  explicit FakeEventServer(const QByteArray& answer) : answer_(answer) {
    listener_.listen(QHostAddress::LocalHost);
    QObject::connect(&listener_, &QTcpServer::newConnection, [this]() {
      while (listener_.hasPendingConnections()) {
        QTcpSocket* socket = listener_.nextPendingConnection();
        auto buffer = std::make_shared<QByteArray>();
        QObject::connect(socket, &QTcpSocket::readyRead,
                         [this, socket, buffer]() {
                           *buffer += socket->readAll();
                           const int end = buffer->indexOf("\r\n\r\n");
                           if (end == -1) return;
                           requests_ << buffer->left(end);
                           buffer->clear();
                           socket->write(answer_);
                           socket->disconnectFromHost();
                         });
      }
    });
  }

  QUrl url() const {
    return QUrl(QString("http://127.0.0.1:%1/event").arg(listener_.serverPort()));
  }
  QList<QByteArray> requests_;

 private:
  QTcpServer listener_;
  QByteArray answer_;
};

QByteArray Header(const QByteArray& request, const QByteArray& name) {
  for (const QByteArray& line : request.split('\n')) {
    if (line.toLower().startsWith(name.toLower() + ":")) {
      return line.mid(name.size() + 1).trimmed();
    }
  }
  return QByteArray();
}

TEST(UpnpSubscriptionTest, SubscribesRenewsAndUnsubscribes) {
  // Renewed a fifth of the second before it runs out.
  FakeEventServer renderer(
      "HTTP/1.1 200 OK\r\nSID: uuid:RINCON_1_sub7\r\nTIMEOUT: Second-1\r\n"
      "Content-Length: 0\r\nConnection: close\r\n\r\n");
  QNetworkAccessManager network;
  auto callbacks = std::make_shared<EventCallbackTable>();
  auto subscription = std::make_unique<UpnpSubscription>(
      &network, callbacks, renderer.url(), QHostAddress::LocalHost);
  QSignalSpy subscribed(subscription.get(), &UpnpSubscription::Subscribed);
  subscription->Start(QUrl("http://192.168.86.214:5500"));
  ASSERT_TRUE(subscribed.wait(5000));
  EXPECT_TRUE(subscription->is_subscribed());

  ASSERT_EQ(1, renderer.requests_.size());
  const QByteArray first = renderer.requests_[0];
  EXPECT_TRUE(first.startsWith("SUBSCRIBE /event HTTP/1.1")) << first.data();
  EXPECT_EQ("upnp:event", Header(first, "NT").toStdString());
  EXPECT_EQ("Second-1800", Header(first, "TIMEOUT").toStdString());
  const QByteArray callback = Header(first, "CALLBACK");
  EXPECT_TRUE(callback.startsWith("<http://192.168.86.214:5500/e/"))
      << callback.data();
  // Which the media server takes events at.
  const QByteArray token = callback.mid(30, callback.size() - 31);
  QHostAddress peer;
  EXPECT_TRUE(callbacks->Find(token, &peer));
  EXPECT_EQ(QHostAddress(QHostAddress::LocalHost), peer);

  ASSERT_TRUE(QTest::qWaitFor([&]() { return renderer.requests_.size() >= 2; },
                              10000));
  const QByteArray renewal = renderer.requests_[1];
  EXPECT_TRUE(renewal.startsWith("SUBSCRIBE /event HTTP/1.1"));
  EXPECT_EQ("uuid:RINCON_1_sub7", Header(renewal, "SID").toStdString());
  EXPECT_TRUE(Header(renewal, "CALLBACK").isEmpty());
  EXPECT_EQ(1, subscribed.size());

  const int before = renderer.requests_.size();
  subscription.reset();
  EXPECT_FALSE(callbacks->Find(token, &peer));
  ASSERT_TRUE(QTest::qWaitFor(
      [&]() { return renderer.requests_.size() > before; }, 10000));
  EXPECT_TRUE(renderer.requests_.last().startsWith("UNSUBSCRIBE /event"));
  EXPECT_EQ("uuid:RINCON_1_sub7",
            Header(renderer.requests_.last(), "SID").toStdString());
}

TEST(UpnpSubscriptionTest, Fails) {
  FakeEventServer renderer(
      "HTTP/1.1 412 Precondition Failed\r\nContent-Length: 0\r\n"
      "Connection: close\r\n\r\n");
  QNetworkAccessManager network;
  auto callbacks = std::make_shared<EventCallbackTable>();
  UpnpSubscription subscription(&network, callbacks, renderer.url(),
                                QHostAddress::LocalHost);
  QSignalSpy failed(&subscription, &UpnpSubscription::Failed);
  subscription.Start(QUrl("http://192.168.86.214:5500"));
  ASSERT_TRUE(failed.wait(5000));
  EXPECT_EQ("HTTP 412", failed[0][0].toString());
  EXPECT_FALSE(subscription.is_subscribed());
}

TEST(UpnpMediaPlayerTest, EventsSayWhatPollsWould) {
  FakePlayer player;
  StartPlaying(&player);
  EXPECT_EQ(UpnpMediaPlayer::kPollIntervalMsec, player.poll_interval_msec());

  // Paused from the Sonos app: no poll needed to notice.
  const int sent = player.sent_.size();
  player.TransportEvent({{"TransportState", "PAUSED_PLAYBACK"}});
  EXPECT_EQ(UpnpMediaPlayer::Paused, player.state());
  player.TransportEvent({{"TransportState", "PLAYING"}});
  EXPECT_EQ(UpnpMediaPlayer::Playing, player.state());
  EXPECT_EQ(sent, player.sent_.size());
  // Polled less, now that they come.
  EXPECT_EQ(UpnpMediaPlayer::kEventPollIntervalMsec,
            player.poll_interval_msec());
  player.Unsubscribe();
  EXPECT_EQ(UpnpMediaPlayer::kPollIntervalMsec, player.poll_interval_msec());
}

TEST(UpnpMediaPlayerTest, EventsAboutWhatPlayedBefore) {
  FakePlayer player;
  player.Load(Info(), 0, true);
  player.Answer();  // SetAVTransportURI
  player.Answer();  // Play

  // Still the last track, as the renderer moves on.
  player.TransportEvent({{"TransportState", "PLAYING"},
                         {"CurrentTrackURI", "http://10.0.0.1:5500/s/old/9"}});
  EXPECT_EQ(UpnpMediaPlayer::Buffering, player.state());
  // A stop before it starts is left to the polls.
  player.TransportEvent({{"TransportState", "STOPPED"},
                         {"CurrentTrackURI", FakePlayer::kUrl}});
  EXPECT_TRUE(player.is_loaded());
  EXPECT_EQ(UpnpMediaPlayer::Buffering, player.state());

  player.TransportEvent({{"TransportState", "PLAYING"},
                         {"CurrentTrackURI", FakePlayer::kUrl}});
  EXPECT_EQ(UpnpMediaPlayer::Playing, player.state());
}

TEST(UpnpMediaPlayerTest, EventsSaySomethingElsePlays) {
  FakePlayer player;
  QSignalSpy lost(&player, &UpnpMediaPlayer::Lost);
  StartPlaying(&player);
  player.TransportEvent(
      {{"TransportState", "PLAYING"},
       {"CurrentTrackURI", "x-sonos-spotify:spotify%3atrack%3a1"}});
  EXPECT_EQ(1, lost.size());
  EXPECT_FALSE(player.is_loaded());
}

TEST(UpnpMediaPlayerTest, EventsSayTheNextStarted) {
  FakePlayer player;
  QSignalSpy next(&player, &UpnpMediaPlayer::NextStarted);
  StartPlaying(&player);
  UpnpMediaInfo info = Info();
  info.url = QUrl("http://10.0.0.1:5500/s/token/2");
  player.SetNext(info);
  player.Answer();

  player.TransportEvent({{"TransportState", "PLAYING"},
                         {"CurrentTrackURI", "http://10.0.0.1:5500/s/token/2"},
                         {"CurrentTrackDuration", "0:03:00"}});
  EXPECT_EQ(1, next.size());
  EXPECT_EQ(180000, player.duration_ms());
}

TEST(UpnpMediaPlayerTest, StoppedByEventAtTheEnd) {
  FakePlayer player;
  QSignalSpy ended(&player, &UpnpMediaPlayer::TrackEnded);
  QSignalSpy lost(&player, &UpnpMediaPlayer::Lost);
  StartPlaying(&player);
  // Polled last a little short of the margin; it played on since.
  player.Poll("0:03:56", "PLAYING");
  QTest::qWait(1100);
  player.TransportEvent({{"TransportState", "STOPPED"}});
  EXPECT_EQ(1, ended.size());
  EXPECT_EQ(0, lost.size());
}

TEST(UpnpMediaPlayerTest, StoppedByEventPartWay) {
  FakePlayer player;
  QSignalSpy ended(&player, &UpnpMediaPlayer::TrackEnded);
  QSignalSpy lost(&player, &UpnpMediaPlayer::Lost);
  StartPlaying(&player);
  player.Poll("0:01:00", "PLAYING");
  player.TransportEvent({{"TransportState", "STOPPED"}});
  EXPECT_EQ(0, ended.size());
  EXPECT_EQ(1, lost.size());
}

TEST(UpnpMediaPlayerTest, VolumeEvents) {
  FakePlayer player;
  QSignalSpy volumes(&player, &UpnpMediaPlayer::RendererVolumeChanged);
  // The group's on Sonos, otherwise the renderer's own.
  player.VolumeEvent(UpnpSubscription::ParsePropertySet(kGroupVolumeEvent));
  player.VolumeEvent(
      UpnpSubscription::ParsePropertySet(kRenderingControlEvent));
  player.VolumeEvent({{"Mute", "1"}});
  ASSERT_EQ(2, volumes.size());
  EXPECT_EQ(20, volumes[0][0].toInt());
  EXPECT_EQ(20, volumes[1][0].toInt());
}

}  // namespace
