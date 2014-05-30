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

#include <memory>

#include "core/logging.h"
#include "musicbrainz/musicbrainzclient.h"

#include <QCoreApplication>
#include <QEventLoop>
#include <QFile>
#include <QMetaType>
#include <QSignalSpy>
#include <QString>

#include "mock_networkaccessmanager.h"
#include "gtest/gtest.h"
#include "test_utils.h"

namespace {
typedef QList<MusicBrainzClient::Result> ResultList;
Q_DECLARE_METATYPE(ResultList);
};

class MusicBrainzClientTest : public ::testing::Test {
 protected:
  void SetUp() {
    mock_network_.reset(new MockNetworkAccessManager);
    qRegisterMetaType<ResultList>("MusicBrainzClient::ResultList");
  }


  // Reads the data from a file into a QByteArray and returns it.
  QByteArray ReadDataFromFile(const QString& filename) {
    QFile file(filename);
    file.open(QIODevice::ReadOnly);
    QByteArray data = file.readAll();
    return data;
  }

  std::unique_ptr<MockNetworkAccessManager> mock_network_;
};


// Test if a discid that do not exist in the musicbrainz database
// generates an empty result.
TEST_F(MusicBrainzClientTest, DiscIdNotFound) {
  QByteArray data =
      "<?xml version=\"1.0\" encoding=\"UTF-8\"?><error><text>Not "
      "Found</text><text>For usage, please see: "
      "http://musicbrainz.org/development/mmd</text></error>";

  // Create a MusicBrainzClient instance with mock_network_.
  MusicBrainzClient musicbrainz_client(nullptr, mock_network_.get());

  // Hook the data as the response to a query of a given type.
  QMap<QString, QString> params;
  params["inc"] = "artists+recordings";
  MockNetworkReply* discid_reply =
      mock_network_->ExpectGet("discid", params, 200, data);

  // Set up a QSignalSpy which stores the result.
  QSignalSpy spy(&musicbrainz_client,
                 SIGNAL(Finished(const QString&, const QString,
                                 const MusicBrainzClient::ResultList&)));
  ASSERT_TRUE(spy.isValid());
  EXPECT_EQ(0, spy.count());

  // Start the request and get a result. The argument doesn't matter
  // in the test.
  musicbrainz_client.StartDiscIdRequest("fooDiscid");
  discid_reply->Done();
  QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
  EXPECT_EQ(1, spy.count());

  QList<QVariant> result = spy.takeFirst();
  QString artist = result.takeFirst().toString();
  QString album = result.takeFirst().toString();
  ResultList tracks = result.takeFirst().value<ResultList>();

  // Check that title and artist are empty, and that there are zero tracks.
  EXPECT_TRUE(artist.isEmpty());
  EXPECT_TRUE(album.isEmpty());
  EXPECT_EQ(0, tracks.count());
}

// Test if MusicBrainzClient::StartDiscIdRequest() parses a discid
// correctly.
TEST_F(MusicBrainzClientTest, ParseDiscID) {
  QByteArray data = ReadDataFromFile(":testdata/discid_2cd.xml");
  ASSERT_FALSE(data.isEmpty());

  // The following are the expected values given for the test file
  // discid_2cd.xml. The discid corresponds to the 2nd disc in the
  // set. The test file contains two releases but we only parse the first.
  const QString expected_artist = "Symphony X";
  const QString expected_title = "Live on the Edge of Forever";
  const int expected_number_of_tracks = 6;

  // Create a MusicBrainzClient instance with mock_network_.
  MusicBrainzClient musicbrainz_client(nullptr, mock_network_.get());

  // Hook the data as the response to a query of a given type.
  QMap<QString, QString> params;
  params["inc"] = "artists+recordings";
  MockNetworkReply* discid_reply =
      mock_network_->ExpectGet("discid", params, 200, data);

  // Set up a QSignalSpy which stores the result.
  QSignalSpy spy(&musicbrainz_client,
                 SIGNAL(Finished(const QString&, const QString,
                                 const MusicBrainzClient::ResultList&)));
  ASSERT_TRUE(spy.isValid());
  EXPECT_EQ(0, spy.count());

  // Start the request and get a result. The argument doesn't matter
  // in the test. It is here set to the discid of the requested disc.
  musicbrainz_client.StartDiscIdRequest("lvcH9_vbw_rJAbXieTOo1CbyNmQ-");
  discid_reply->Done();
  QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
  EXPECT_EQ(1, spy.count());

  QList<QVariant> result = spy.takeFirst();
  QString artist = result.takeFirst().toString();
  QString album = result.takeFirst().toString();
  ResultList tracks = result.takeFirst().value<ResultList>();

  // Check that title and artist are correct.
  EXPECT_EQ(expected_artist, artist);
  EXPECT_EQ(expected_title, album);

  // Check that we get the correct number of tracks, i.e. that the
  // correct disc is chosen in a multi-disc release.
  EXPECT_EQ(expected_number_of_tracks, tracks.count());

  // Check that the tracks is ordered by track number in ascending
  // order.
  for (int i = 0; i < tracks.count(); ++i) {
    EXPECT_EQ(i + 1, tracks[i].track_);
  }

  // Check some track information.
  EXPECT_EQ("Smoke and Mirrors", tracks[0].title_);
  EXPECT_EQ(1, tracks[0].track_);
  EXPECT_EQ(394600, tracks[0].duration_msec_);

  EXPECT_EQ("Church of the Machine", tracks[1].title_);
  EXPECT_EQ(2, tracks[1].track_);
  EXPECT_EQ(441866, tracks[1].duration_msec_);
}

// Test if MusicBrainzClient::Start() parses a track correctly.
TEST_F(MusicBrainzClientTest, ParseTrack) {
  QByteArray data = ReadDataFromFile(":testdata/recording.xml");
  ASSERT_FALSE(data.isEmpty());

  // Expected results from the test file recording.xml:
  const int expected_track_number = 6;
  const QString expected_title = "Victoria und ihr Husar: Pardon Madame";
  const QString expected_artist = "Paul Abraham";
  const QString expected_album = "An Evening at the Operetta";

  // Create a MusicBrainzClient instance with mock_network_.
  MusicBrainzClient musicbrainz_client(nullptr, mock_network_.get());

  // Hook the data as the response to a query of a given type.
  QMap<QString, QString> params;
  params["inc"] = "artists+releases+media";
  MockNetworkReply* discid_reply =
      mock_network_->ExpectGet("recording", params, 200, data);

  QSignalSpy spy(&musicbrainz_client,
                 SIGNAL(Finished(int, const MusicBrainzClient::ResultList&)));
  ASSERT_TRUE(spy.isValid());
  EXPECT_EQ(0, spy.count());

  // Start the request and get a result.
  // The mbid argument doesn't matter in the test.
  const int sent_id = 0;
  musicbrainz_client.Start(sent_id, "fooMbid");
  discid_reply->Done();
  QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
  EXPECT_EQ(1, spy.count());

  QList<QVariant> result = spy.takeFirst();
  int id = result.takeFirst().toInt();
  EXPECT_EQ(sent_id, id);

  ResultList tracks = result.takeFirst().value<ResultList>();
  for (const MusicBrainzClient::Result& track : tracks) {
    EXPECT_EQ(expected_track_number, track.track_);
    EXPECT_EQ(expected_title, track.title_);
    EXPECT_EQ(expected_artist, track.artist_);
    EXPECT_EQ(expected_album, track.album_);
  }
}

// For a recording with multiple releases, we should get them all.
TEST_F(MusicBrainzClientTest, ParseTrackWithMultipleReleases) {
  QByteArray data =
      ReadDataFromFile(":testdata/recording_with_multiple_releases.xml");
  ASSERT_FALSE(data.isEmpty());

  const int expected_number_of_releases = 7;

  // Create a MusicBrainzClient instance with mock_network_.
  MusicBrainzClient musicbrainz_client(nullptr, mock_network_.get());

  // Hook the data as the response to a query of a given type.
  QMap<QString, QString> params;
  params["inc"] = "artists+releases+media";
  MockNetworkReply* discid_reply =
      mock_network_->ExpectGet("recording", params, 200, data);

  QSignalSpy spy(&musicbrainz_client,
                 SIGNAL(Finished(int, const MusicBrainzClient::ResultList&)));
  ASSERT_TRUE(spy.isValid());
  EXPECT_EQ(0, spy.count());

  // Start the request and get a result.
  // The mbid argument doesn't matter in the test.
  const int sent_id = 0;
  musicbrainz_client.Start(sent_id, "fooMbid");
  discid_reply->Done();
  QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
  EXPECT_EQ(1, spy.count());

  QList<QVariant> result = spy.takeFirst();
  int id = result.takeFirst().toInt();
  EXPECT_EQ(sent_id, id);

  ResultList tracks = result.takeFirst().value<ResultList>();
  EXPECT_EQ(expected_number_of_releases, tracks.count());
}
