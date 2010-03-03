#include "albumcoverfetcher.h"

#include <lastfm/ws.h>

#include <QCoreApplication>
#include <QEventLoop>
#include <QSignalSpy>

#include "mock_networkaccessmanager.h"
#include "gtest/gtest.h"

int argc = 1;
char* argv[] = { "test", 0 };

class AlbumCoverFetcherTest : public ::testing::Test {
 protected:
  static void SetUpTestCase() {
    lastfm::ws::ApiKey = "foobar";
  }

  AlbumCoverFetcherTest()
      : app_(argc, (char**)argv) {
  }

  void SetUp() {
    // Lastfm takes ownership of this.
    network_ = new MockNetworkAccessManager;
    lastfm::setNetworkAccessManager(network_);
  }

  MockNetworkAccessManager* network_;
  QCoreApplication app_;
};


TEST_F(AlbumCoverFetcherTest, FetchesAlbumCover) {
  const char* data = "<lfm status=\"ok\"><album><name>Bar</name><artist>Foo</artist>"
                     "<image size=\"large\">http://example.com/image.jpg</image></album></lfm>";

  QMap<QString, QString> params;
  params["artist"] = "Foo";
  params["album"] = "Bar";
  params["api_key"] = "foobar";
  MockNetworkReply* get_info_reply = network_->ExpectGet("audioscrobbler", params, 200, data);
  params.clear();
  MockNetworkReply* album_reply = network_->ExpectGet("http://example.com/image.jpg", params, 200, "");

  AlbumCoverFetcher fetcher(network_, NULL);
  QSignalSpy spy(&fetcher, SIGNAL(AlbumCoverFetched(quint64, const QImage&)));
  ASSERT_TRUE(spy.isValid());
  fetcher.FetchAlbumCover("Foo", "Bar");

  get_info_reply->Done();
  app_.processEvents(QEventLoop::AllEvents);

  album_reply->Done();
  app_.processEvents(QEventLoop::AllEvents);

  EXPECT_EQ(1, spy.count());
}
