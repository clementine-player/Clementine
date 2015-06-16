/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

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

#include "core/albumcoverfetcher.h"
#include "core/networkaccessmanager.h"

#include <lastfm5/ws.h>

#include <QCoreApplication>
#include <QEventLoop>
#include <QSignalSpy>

#include "mock_networkaccessmanager.h"
#include "gtest/gtest.h"

namespace {

class AlbumCoverFetcherTest : public ::testing::Test {
 protected:
  static void SetUpTestCase() {
    lastfm::ws::ApiKey = "foobar";

    // Lastfm takes ownership of this.
    mock_network_ = new MockNetworkAccessManager;
    lastfm::setNetworkAccessManager(mock_network_);
  }

  void SetUp() {
    network_ = new NetworkAccessManager(nullptr, mock_network_);
  }

  void TearDown() {
    delete network_;
  }

  static void TearDownTestCase() {
    delete mock_network_;
  }

  static MockNetworkAccessManager* mock_network_;
  NetworkAccessManager* network_;
};

MockNetworkAccessManager* AlbumCoverFetcherTest::mock_network_;


TEST_F(AlbumCoverFetcherTest, FetchesAlbumCover) {
  QByteArray data("<lfm status=\"ok\"><album><name>Bar</name><artist>Foo</artist>"
                  "<image size=\"large\">http://example.com/image.jpg</image></album></lfm>");

  QMap<QString, QString> params;
  params["artist"] = "Foo";
  params["album"] = "Bar";
  params["api_key"] = "foobar";
  params["method"] = "album.getInfo";
  MockNetworkReply* get_info_reply = mock_network_->ExpectGet("audioscrobbler", params, 200, data);
  params.clear();
  MockNetworkReply* album_reply = mock_network_->ExpectGet("http://example.com/image.jpg", params, 200, "");

  AlbumCoverFetcher fetcher(network_, nullptr);
  QSignalSpy spy(&fetcher, SIGNAL(AlbumCoverFetched(quint64, const QImage&)));
  ASSERT_TRUE(spy.isValid());
  fetcher.FetchAlbumCover("Foo", "Bar");

  get_info_reply->Done();
  QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

  album_reply->Done();
  QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

  EXPECT_EQ(1, spy.count());
}

}  // namespace
