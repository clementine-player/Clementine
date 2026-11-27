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

#include "internet/podcasts/gpodderclient.h"

#include <QIODevice>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QUrlQuery>
#include <memory>

#include "gtest/gtest.h"
#include "mock_networkaccessmanager.h"
#include "test_utils.h"

// For the values in failure messages; test_utils.h has the rest. These are
// found by argument-dependent lookup, so they're in the global namespace.
std::ostream& operator<<(std::ostream& stream, const QByteArray& data) {
  return stream << data.toStdString();
}

std::ostream& operator<<(std::ostream& stream,
                         GPodderClient::EpisodeAction::Status status) {
  return stream << static_cast<int>(status);
}

namespace {

// Records each request the client makes and answers it with the next reply
// queued with Reply().
class FakeNetworkAccessManager : public QNetworkAccessManager {
 public:
  struct Sent {
    Operation operation;
    QNetworkRequest request;
    QByteArray body;
  };

  // The reply to the next request. Call Done() on it to finish it.
  MockNetworkReply* Reply(int status, const QByteArray& data) {
    MockNetworkReply* reply = new MockNetworkReply(data);
    reply->setAttribute(QNetworkRequest::HttpStatusCodeAttribute, status);
    replies_ << reply;
    return reply;
  }

  QList<Sent> sent;

 protected:
  QNetworkReply* createRequest(Operation op, const QNetworkRequest& request,
                               QIODevice* body) override {
    sent << Sent{op, request, body ? body->readAll() : QByteArray()};
    if (replies_.isEmpty()) {
      ADD_FAILURE() << "Unexpected request for "
                    << request.url().toString().toStdString();
      Reply(500, QByteArray());
    }
    return replies_.takeFirst();
  }

 private:
  QList<MockNetworkReply*> replies_;
};

class GPodderClientTest : public ::testing::Test {
 protected:
  GPodderClientTest() : client_(&network_) {}

  // Fails the test if called.
  static GPodderClient::ErrorCallback NoError() {
    return [](const QString& error) {
      ADD_FAILURE() << "Unexpected error: " << error.toStdString();
    };
  }

  QJsonObject SentJson(int i) const {
    return QJsonDocument::fromJson(network_.sent[i].body).object();
  }

  FakeNetworkAccessManager network_;
  GPodderClient client_;
};

const char kPodcastJson[] = R"({
  "url": "https://example.com/feed.xml",
  "title": "A podcast",
  "description": "About things",
  "subscribers": 42,
  "subscribers_last_week": 40,
  "logo_url": "https://example.com/logo.png",
  "website": "https://example.com/",
  "mygpo_link": "https://gpodder.net/podcast/a-podcast"
})";

TEST_F(GPodderClientTest, SearchParsesPodcasts) {
  MockNetworkReply* reply =
      network_.Reply(200, QByteArray("[") + kPodcastJson + "]");

  QList<Podcast> result;
  client_.Search(
      "rock & roll",
      [&result](const QList<Podcast>& podcasts) { result = podcasts; },
      NoError());

  ASSERT_EQ(1, network_.sent.count());
  const QUrl url = network_.sent[0].request.url();
  EXPECT_EQ(QNetworkAccessManager::GetOperation, network_.sent[0].operation);
  EXPECT_EQ("https://gpodder.net/search.json", url.toString(QUrl::RemoveQuery));
  EXPECT_EQ("rock & roll",
            QUrlQuery(url).queryItemValue("q", QUrl::FullyDecoded));
  // The directory doesn't need an account.
  EXPECT_FALSE(network_.sent[0].request.hasRawHeader("Authorization"));

  reply->Done();

  ASSERT_EQ(1, result.count());
  const Podcast& podcast = result[0];
  EXPECT_EQ(QUrl("https://example.com/feed.xml"), podcast.url());
  EXPECT_EQ("A podcast", podcast.title());
  EXPECT_EQ("About things", podcast.description());
  EXPECT_EQ(QUrl("https://example.com/"), podcast.link());
  EXPECT_EQ(QUrl("https://example.com/logo.png"), podcast.image_url_large());
  EXPECT_EQ(42, podcast.extra("gpodder:subscribers").toInt());
  EXPECT_EQ(40, podcast.extra("gpodder:subscribers_last_week").toInt());
  EXPECT_EQ(QUrl("https://gpodder.net/podcast/a-podcast"),
            podcast.extra("gpodder:page").toUrl());
}

TEST_F(GPodderClientTest, TopTags) {
  MockNetworkReply* reply =
      network_.Reply(200, R"([{"tag": "technology", "usage": 530},
               {"tag": "music", "usage": 410}])");

  QStringList result;
  client_.TopTags(
      100, [&result](const QStringList& tags) { result = tags; }, NoError());

  ASSERT_EQ(1, network_.sent.count());
  EXPECT_EQ(QUrl("https://gpodder.net/api/2/tags/100.json"),
            network_.sent[0].request.url());

  reply->Done();
  EXPECT_EQ(QStringList() << "technology"
                          << "music",
            result);
}

TEST_F(GPodderClientTest, PodcastsOfTagEncodesTheTag) {
  MockNetworkReply* reply =
      network_.Reply(200, QByteArray("[") + kPodcastJson + "]");

  int count = -1;
  client_.PodcastsOfTag(
      "news/politics", 50,
      [&count](const QList<Podcast>& podcasts) { count = podcasts.count(); },
      NoError());

  ASSERT_EQ(1, network_.sent.count());
  EXPECT_EQ("https://gpodder.net/api/2/tag/news%2Fpolitics/50.json",
            network_.sent[0].request.url().toEncoded().toStdString());

  reply->Done();
  EXPECT_EQ(1, count);
}

TEST_F(GPodderClientTest, UpdateDeviceSendsCredentialsAndCaption) {
  client_.SetCredentials("alice", "s3cret");
  MockNetworkReply* reply = network_.Reply(200, "");

  bool done = false;
  client_.UpdateDevice(
      "clementine-host", "Clementine on host",
      GPodderClient::DeviceType::kLaptop, [&done]() { done = true; },
      NoError());

  ASSERT_EQ(1, network_.sent.count());
  const FakeNetworkAccessManager::Sent& sent = network_.sent[0];
  EXPECT_EQ(QNetworkAccessManager::PostOperation, sent.operation);
  EXPECT_EQ(
      QUrl("https://gpodder.net/api/2/devices/alice/clementine-host.json"),
      sent.request.url());
  EXPECT_EQ("Basic " + QByteArray("alice:s3cret").toBase64(),
            sent.request.rawHeader("Authorization"));
  EXPECT_EQ("application/json",
            sent.request.header(QNetworkRequest::ContentTypeHeader).toString());
  EXPECT_EQ("Clementine on host", SentJson(0)["caption"].toString());
  EXPECT_EQ("laptop", SentJson(0)["type"].toString());

  reply->Done();
  EXPECT_TRUE(done);
}

TEST_F(GPodderClientTest, WrongPasswordIsAnError) {
  client_.SetCredentials("alice", "wrong");
  MockNetworkReply* reply = network_.Reply(401, "");

  QString error;
  client_.UpdateDevice(
      "clementine-host", "Clementine on host",
      GPodderClient::DeviceType::kDesktop,
      []() { ADD_FAILURE() << "Unexpected success"; },
      [&error](const QString& e) { error = e; });
  EXPECT_EQ("desktop", SentJson(0)["type"].toString());

  reply->Done();
  EXPECT_FALSE(error.isEmpty());
}

TEST_F(GPodderClientTest, GetDeviceUpdates) {
  client_.SetCredentials("alice", "s3cret");
  MockNetworkReply* reply = network_.Reply(200, QByteArray(R"({
    "add": [)") + kPodcastJson + R"(],
    "remove": ["https://example.com/old.xml"],
    "updates": [
      {"podcast_url": "https://example.com/feed.xml",
       "url": "https://example.com/1.mp3", "status": "play"},
      {"podcast_url": "https://example.com/feed.xml",
       "url": "https://example.com/2.mp3", "status": "download"},
      {"podcast_url": "https://example.com/feed.xml",
       "url": "https://example.com/3.mp3", "status": "new"},
      {"podcast_url": "https://example.com/feed.xml",
       "url": "https://example.com/4.mp3", "status": "something"}
    ],
    "timestamp": 1700000000
  })");

  std::unique_ptr<GPodderClient::DeviceUpdates> result;
  client_.GetDeviceUpdates(
      "clementine-host", 1600000000,
      [&result](const GPodderClient::DeviceUpdates& updates) {
        result.reset(new GPodderClient::DeviceUpdates(updates));
      },
      NoError());

  ASSERT_EQ(1, network_.sent.count());
  const QUrl url = network_.sent[0].request.url();
  EXPECT_EQ("https://gpodder.net/api/2/updates/alice/clementine-host.json",
            url.toString(QUrl::RemoveQuery));
  EXPECT_EQ("1600000000", QUrlQuery(url).queryItemValue("since"));

  reply->Done();
  ASSERT_TRUE(result != nullptr);

  ASSERT_EQ(1, result->add.count());
  EXPECT_EQ(QUrl("https://example.com/feed.xml"), result->add[0].url());
  EXPECT_EQ(QList<QUrl>() << QUrl("https://example.com/old.xml"),
            result->remove);

  using Status = GPodderClient::EpisodeAction::Status;
  ASSERT_EQ(4, result->updates.count());
  EXPECT_EQ(QUrl("https://example.com/feed.xml"),
            result->updates[0].podcast_url);
  EXPECT_EQ(QUrl("https://example.com/1.mp3"), result->updates[0].url);
  EXPECT_EQ(Status::kPlayed, result->updates[0].status);
  EXPECT_EQ(Status::kDownloaded, result->updates[1].status);
  EXPECT_EQ(Status::kNew, result->updates[2].status);
  EXPECT_EQ(Status::kUnknown, result->updates[3].status);
}

TEST_F(GPodderClientTest, AddRemoveSubscriptions) {
  client_.SetCredentials("alice", "s3cret");
  MockNetworkReply* reply =
      network_.Reply(200, R"({"timestamp": 1700000000, "update_urls": []})");

  bool done = false;
  client_.AddRemoveSubscriptions(
      "clementine-host", QList<QUrl>() << QUrl("https://example.com/new.xml"),
      QList<QUrl>() << QUrl("https://example.com/old.xml"),
      [&done]() { done = true; }, NoError());

  ASSERT_EQ(1, network_.sent.count());
  EXPECT_EQ(QNetworkAccessManager::PostOperation, network_.sent[0].operation);
  EXPECT_EQ(QUrl("https://gpodder.net/api/2/subscriptions/alice/"
                 "clementine-host.json"),
            network_.sent[0].request.url());
  const QJsonObject body = SentJson(0);
  ASSERT_EQ(1, body["add"].toArray().count());
  EXPECT_EQ("https://example.com/new.xml", body["add"].toArray()[0].toString());
  ASSERT_EQ(1, body["remove"].toArray().count());
  EXPECT_EQ("https://example.com/old.xml",
            body["remove"].toArray()[0].toString());

  reply->Done();
  EXPECT_TRUE(done);
}

TEST_F(GPodderClientTest, UnreadableReplyIsAnError) {
  MockNetworkReply* reply = network_.Reply(200, "<html>Down for a bit</html>");

  QString error;
  client_.TopTags(
      10, [](const QStringList&) { ADD_FAILURE() << "Unexpected success"; },
      [&error](const QString& e) { error = e; });

  reply->Done();
  EXPECT_FALSE(error.isEmpty());
}

}  // namespace
