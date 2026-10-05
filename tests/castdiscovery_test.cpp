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

#include "chromecast/castdiscovery.h"

#include <QDebug>

#include "gtest/gtest.h"
#include "test_utils.h"

std::ostream& operator<<(std::ostream& stream, const CastDevice& device) {
  QString text;
  QDebug(&text) << device;
  return stream << text.toStdString();
}

namespace {

// A TXT record like a Chromecast Ultra's.
QList<QByteArray> Txt(const QByteArray& id, const QByteArray& name) {
  return {"rs=",
          "nf=1",
          "ca=201221",
          "fn=" + name,
          "md=Chromecast Ultra",
          "ve=05",
          "id=" + id,
          "ic=/setup/icon.png"};
}

QString Joined(const QList<QByteArray>& entries) {
  return QString::fromUtf8(entries.join('|'));
}

CastDevice Device(const QByteArray& id, const QByteArray& name,
                  const QString& address) {
  CastDevice device;
  EXPECT_TRUE(device.ParseTxt(Txt(id, name)));
  device.address = QHostAddress(address);
  device.port = 8009;
  return device;
}

class FakeCastDiscovery : public CastDiscovery {
 public:
  FakeCastDiscovery() {
    connect(this, &CastDiscovery::DeviceFound,
            [this](const CastDevice& device) { found_ << device; });
    connect(this, &CastDiscovery::DeviceLost,
            [this](const QString& id) { lost_ << id; });
  }

  void Start() override {}

  using CastDiscovery::ServiceAdded;
  using CastDiscovery::ServiceRemoved;
  using CastDiscovery::ServiceResolved;

  // An advertisement that appears and resolves straight away.
  void Resolve(const QString& service, const CastDevice& device) {
    ServiceAdded(service);
    ServiceResolved(service, device);
  }

  QList<CastDevice> found_;
  QStringList lost_;
};

TEST(CastDeviceTest, ParsesTxt) {
  CastDevice device;
  ASSERT_TRUE(device.ParseTxt(Txt("ba6eb2c5", "Living Room TV")));
  EXPECT_EQ("ba6eb2c5", device.id);
  EXPECT_EQ("Living Room TV", device.name);
  EXPECT_EQ("Chromecast Ultra", device.model);
}

TEST(CastDeviceTest, ValueMayContainEquals) {
  CastDevice device;
  ASSERT_TRUE(device.ParseTxt({"id=abc", "fn=a=b"}));
  EXPECT_EQ("a=b", device.name);
}

TEST(CastDeviceTest, NameFallsBackToModel) {
  CastDevice device;
  ASSERT_TRUE(device.ParseTxt({"id=abc", "md=Google Nest Mini"}));
  EXPECT_EQ("Google Nest Mini", device.name);
}

TEST(CastDeviceTest, NeedsAnId) {
  CastDevice device;
  EXPECT_FALSE(device.ParseTxt({"fn=Kitchen", "md=Google Nest Hub", "id="}));
  EXPECT_FALSE(device.ParseTxt({}));
  EXPECT_TRUE(device.id.isEmpty());
}

TEST(CastDeviceTest, SplitsTxt) {
  const QByteArray rdata(
      "\x06id=abc\x00\x0a"
      "fn=Kitchen\x03"
      "rs=",
      23);
  EXPECT_EQ("id=abc|fn=Kitchen|rs=", Joined(CastDevice::SplitTxt(rdata)));
}

TEST(CastDeviceTest, SplitTxtDropsATruncatedEntry) {
  EXPECT_EQ("id=abc", Joined(CastDevice::SplitTxt(QByteArray("\x06id=abc\x09"
                                                             "fn=K",
                                                             12))));
  EXPECT_TRUE(CastDevice::SplitTxt(QByteArray()).isEmpty());
}

TEST(CastDeviceTest, ValidNeedsAnAddressAndPort) {
  CastDevice device = Device("abc", "Kitchen", "192.168.1.2");
  EXPECT_TRUE(device.is_valid());
  device.port = 0;
  EXPECT_FALSE(device.is_valid());
  device.port = 8009;
  device.address = QHostAddress();
  EXPECT_FALSE(device.is_valid());
}

TEST(CastDiscoveryTest, MergesServicesForOneDevice) {
  FakeCastDiscovery discovery;
  const CastDevice device = Device("abc", "Kitchen", "192.168.1.2");

  // Seen over IPv4 and IPv6 on the same interface.
  discovery.Resolve("2/0/Kitchen", device);
  discovery.Resolve("2/1/Kitchen", device);
  ASSERT_EQ(1, discovery.found_.size());
  EXPECT_EQ(device, discovery.found_[0]);
  EXPECT_EQ(1, discovery.devices().size());

  discovery.ServiceRemoved("2/1/Kitchen");
  EXPECT_TRUE(discovery.lost_.isEmpty());
  EXPECT_EQ(1, discovery.devices().size());

  discovery.ServiceRemoved("2/0/Kitchen");
  EXPECT_EQ("abc", discovery.lost_.join(","));
  EXPECT_TRUE(discovery.devices().isEmpty());
}

TEST(CastDiscoveryTest, ReportsChanges) {
  FakeCastDiscovery discovery;
  discovery.Resolve("2/0/Kitchen", Device("abc", "Kitchen", "192.168.1.2"));
  discovery.Resolve("2/0/Kitchen", Device("abc", "Kitchen", "192.168.1.2"));
  EXPECT_EQ(1, discovery.found_.size());

  discovery.Resolve("2/0/Kitchen", Device("abc", "Kitchen", "192.168.1.3"));
  discovery.Resolve("2/0/Kitchen", Device("abc", "Dining room", "192.168.1.3"));
  ASSERT_EQ(3, discovery.found_.size());
  EXPECT_EQ("Dining room", discovery.found_[2].name);
  EXPECT_EQ(1, discovery.devices().size());
  EXPECT_TRUE(discovery.lost_.isEmpty());
}

TEST(CastDiscoveryTest, KeepsDevicesApart) {
  FakeCastDiscovery discovery;
  discovery.Resolve("2/0/Kitchen", Device("abc", "Kitchen", "192.168.1.2"));
  discovery.Resolve("2/0/Bedroom", Device("def", "Bedroom", "192.168.1.3"));
  EXPECT_EQ(2, discovery.devices().size());

  discovery.ServiceRemoved("2/0/Kitchen");
  EXPECT_EQ("abc", discovery.lost_.join(","));
  ASSERT_EQ(1, discovery.devices().size());
  EXPECT_EQ("def", discovery.devices()[0].id);
}

TEST(CastDiscoveryTest, ServiceThatChangesDeviceLosesTheOldOne) {
  FakeCastDiscovery discovery;
  discovery.Resolve("2/0/Speaker", Device("abc", "Speaker", "192.168.1.2"));
  discovery.Resolve("2/0/Speaker", Device("def", "Speaker", "192.168.1.2"));
  EXPECT_EQ("abc", discovery.lost_.join(","));
  ASSERT_EQ(1, discovery.devices().size());
  EXPECT_EQ("def", discovery.devices()[0].id);
}

TEST(CastDiscoveryTest, IgnoresIncompleteAndUnknownServices) {
  FakeCastDiscovery discovery;
  CastDevice no_address = Device("abc", "Kitchen", "192.168.1.2");
  no_address.address = QHostAddress();
  discovery.Resolve("2/0/Kitchen", no_address);
  EXPECT_TRUE(discovery.found_.isEmpty());

  discovery.ServiceRemoved("2/0/Kitchen");
  discovery.ServiceRemoved("2/0/Unknown");
  EXPECT_TRUE(discovery.lost_.isEmpty());
}

TEST(CastDiscoveryTest, IgnoresResolvesForServicesThatAreGone) {
  FakeCastDiscovery discovery;
  const CastDevice device = Device("abc", "Kitchen", "192.168.1.2");

  discovery.ServiceResolved("2/0/Kitchen", device);
  EXPECT_TRUE(discovery.found_.isEmpty());

  discovery.ServiceAdded("2/0/Kitchen");
  discovery.ServiceRemoved("2/0/Kitchen");
  discovery.ServiceResolved("2/0/Kitchen", device);
  EXPECT_TRUE(discovery.found_.isEmpty());
  EXPECT_TRUE(discovery.devices().isEmpty());
}

}  // namespace
