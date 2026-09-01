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

#include <memory>

#include <QCoreApplication>

#include "internet/googledrive/googledriveclient.h"

#include "mock_networkaccessmanager.h"
#include "gtest/gtest.h"
#include "test_utils.h"

using google_drive::Client;
using google_drive::File;
using google_drive::GetFileResponse;
using google_drive::ListChangesResponse;

class GoogleDriveClientTest : public ::testing::Test {
 protected:
  void SetUp() override { mock_network_.reset(new MockNetworkAccessManager); }

  std::unique_ptr<MockNetworkAccessManager> mock_network_;
};

// GetFile should parse the v3 file resource fields File's accessors read.
TEST_F(GoogleDriveClientTest, GetFileParsesMetadata) {
  Client client(nullptr, mock_network_.get());

  MockNetworkReply* reply = mock_network_->ExpectGet(
      "drive/v3/files/fileA", QMap<QString, QString>(), 200,
      QByteArray(
          "{\"id\": \"fileA\", \"name\": \"Song A.mp3\", \"mimeType\": "
          "\"audio/mpeg\", \"size\": \"12345\", \"version\": \"7\"}"));

  GetFileResponse* response = client.GetFile("fileA");

  bool finished = false;
  QObject::connect(response, &GetFileResponse::Finished,
                   [&]() { finished = true; });

  reply->Done();
  QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

  ASSERT_TRUE(finished);
  EXPECT_FALSE(response->had_error());
  EXPECT_EQ("fileA", response->file().id());
  EXPECT_EQ("Song A.mp3", response->file().title());
  EXPECT_EQ("audio/mpeg", response->file().mime_type());
  EXPECT_EQ(12345, response->file().size());
  EXPECT_EQ("7", response->file().etag());
}

// A Drive API error is still valid JSON (eg. {"error": {...}}), so a
// non-200 status has to be checked before treating a parseable body as
// success - otherwise callers would see an empty-but-"successful" File.
TEST_F(GoogleDriveClientTest, GetFileReportsHttpErrors) {
  Client client(nullptr, mock_network_.get());

  MockNetworkReply* reply = mock_network_->ExpectGet(
      "drive/v3/files/missing", QMap<QString, QString>(), 404,
      QByteArray("{\"error\": {\"code\": 404, \"message\": \"File not "
                "found: missing\"}}"));

  GetFileResponse* response = client.GetFile("missing");

  bool finished = false;
  QObject::connect(response, &GetFileResponse::Finished,
                   [&]() { finished = true; });

  reply->Done();
  QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

  ASSERT_TRUE(finished);
  EXPECT_TRUE(response->had_error());
}

// v3's Changes API has no "list everything since account creation" concept
// - an empty cursor should just establish a starting point via
// changes/startPageToken, with no changes reported.
TEST_F(GoogleDriveClientTest, ListChangesEstablishesStartPageTokenWhenCursorEmpty) {
  Client client(nullptr, mock_network_.get());

  MockNetworkReply* reply = mock_network_->ExpectGet(
      "changes/startPageToken", QMap<QString, QString>(), 200,
      QByteArray("{\"startPageToken\": \"12345\"}"));

  ListChangesResponse* response = client.ListChanges(QString());

  bool found_called = false;
  bool deleted_called = false;
  bool finished = false;
  QObject::connect(response, &ListChangesResponse::FilesFound,
                   [&](const QList<File>&) { found_called = true; });
  QObject::connect(response, &ListChangesResponse::FilesDeleted,
                   [&](const QList<QUrl>&) { deleted_called = true; });
  QObject::connect(response, &ListChangesResponse::Finished,
                   [&]() { finished = true; });

  reply->Done();
  QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

  EXPECT_FALSE(found_called);
  EXPECT_FALSE(deleted_called);
  EXPECT_TRUE(finished);
  EXPECT_EQ("12345", response->next_cursor());
}

// Given a real cursor, ListChanges should classify each change as found or
// deleted (removed, or trashed) and carry the new cursor forward.
TEST_F(GoogleDriveClientTest, ListChangesParsesFoundAndDeletedItems) {
  Client client(nullptr, mock_network_.get());

  QMap<QString, QString> params;
  params["pageToken"] = "cursor1";
  MockNetworkReply* reply = mock_network_->ExpectGet(
      "drive/v3/changes", params, 200,
      QByteArray(
          "{\"changes\": ["
          "{\"fileId\": \"fileA\", \"removed\": false, \"file\": "
          "{\"id\": \"fileA\", \"name\": \"Song A.mp3\", \"mimeType\": "
          "\"audio/mpeg\", \"trashed\": false}},"
          "{\"fileId\": \"fileB\", \"removed\": true}"
          "], \"newStartPageToken\": \"67890\"}"));

  ListChangesResponse* response = client.ListChanges("cursor1");

  QList<File> found;
  QList<QUrl> deleted;
  bool finished = false;
  QObject::connect(response, &ListChangesResponse::FilesFound,
                   [&](const QList<File>& files) { found << files; });
  QObject::connect(response, &ListChangesResponse::FilesDeleted,
                   [&](const QList<QUrl>& urls) { deleted << urls; });
  QObject::connect(response, &ListChangesResponse::Finished,
                   [&]() { finished = true; });

  reply->Done();
  QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

  EXPECT_TRUE(finished);
  EXPECT_EQ("67890", response->next_cursor());
  ASSERT_EQ(1, found.size());
  EXPECT_EQ("fileA", found[0].id());
  ASSERT_EQ(1, deleted.size());
  EXPECT_EQ(QUrl("googledrive:/fileB"), deleted[0]);
}
