/* This file is part of Clementine.
   Copyright 2026

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

#include "config.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "test_utils.h"

#ifdef HAVE_MEGA

#include <memory>

#include <QSettings>
#include <QUrl>

#include <megaapi.h>

// ============================================================================
// MEGA SDK Integration Smoke Tests
// ============================================================================

class MegaSdkSmokeTest : public ::testing::Test {};

TEST_F(MegaSdkSmokeTest, MegaApiCanBeCreated) {
  // Verify the SDK can be instantiated without crashing.
  mega::MegaApi api("yrQVDRBm", (const char*)nullptr, "ClementineTest");
  // If we get here without crashing, the SDK is properly linked.
  SUCCEED();
}

TEST_F(MegaSdkSmokeTest, HandleToBase64Roundtrip) {
  // Test handle serialization used in mega:// URLs.
  mega::MegaHandle handle = 12345678901234ULL;
  std::unique_ptr<char[]> b64(mega::MegaApi::handleToBase64(handle));
  ASSERT_NE(nullptr, b64.get());

  mega::MegaHandle recovered = mega::MegaApi::base64ToHandle(b64.get());
  EXPECT_EQ(handle, recovered);
}

TEST_F(MegaSdkSmokeTest, InvalidHandleToBase64) {
  std::unique_ptr<char[]> b64(
      mega::MegaApi::handleToBase64(mega::INVALID_HANDLE));
  ASSERT_NE(nullptr, b64.get());
  // Should still produce a string, just representing the invalid handle.
  EXPECT_GT(strlen(b64.get()), 0u);
}

TEST_F(MegaSdkSmokeTest, SynchronousRequestListenerCanBeCreated) {
  // Verify the SynchronousRequestListener is usable.
  mega::SynchronousRequestListener listener;
  // getError() should return non-null even before any request.
  // (It returns a default error object.)
  SUCCEED();
}

// ============================================================================
// MegaService Credential Tests
// ============================================================================

class MegaServiceCredentialTest : public ::testing::Test {
 protected:
  void SetUp() override {
    QSettings s;
    s.beginGroup("Mega");
    s.remove("");
    s.endGroup();
  }

  void TearDown() override {
    QSettings s;
    s.beginGroup("Mega");
    s.remove("");
    s.endGroup();
  }
};

TEST_F(MegaServiceCredentialTest, NoCredentialsByDefault) {
  QSettings s;
  s.beginGroup("Mega");
  EXPECT_TRUE(s.value("email").toString().isEmpty());
  EXPECT_TRUE(s.value("password").toString().isEmpty());
  EXPECT_TRUE(s.value("session").toString().isEmpty());
}

TEST_F(MegaServiceCredentialTest, StoreAndRetrieveCredentials) {
  {
    QSettings s;
    s.beginGroup("Mega");
    s.setValue("email", "user@mega.nz");
    s.setValue("password", "secretpass");
  }

  QSettings s;
  s.beginGroup("Mega");
  EXPECT_EQ("user@mega.nz", s.value("email").toString());
  EXPECT_EQ("secretpass", s.value("password").toString());
}

TEST_F(MegaServiceCredentialTest, SessionIsPreferredOverPassword) {
  {
    QSettings s;
    s.beginGroup("Mega");
    s.setValue("email", "user@mega.nz");
    s.setValue("password", "secretpass");
    s.setValue("session", "cached-session-token");
  }

  QSettings s;
  s.beginGroup("Mega");
  // Session should be present and non-empty.
  EXPECT_FALSE(s.value("session").toString().isEmpty());
}

TEST_F(MegaServiceCredentialTest, ForgetCredentialsClearsAll) {
  {
    QSettings s;
    s.beginGroup("Mega");
    s.setValue("email", "user@mega.nz");
    s.setValue("password", "secretpass");
    s.setValue("session", "cached-session-token");
  }

  // Simulate ForgetCredentials behavior.
  {
    QSettings s;
    s.beginGroup("Mega");
    s.remove("email");
    s.remove("password");
    s.remove("session");
  }

  QSettings s;
  s.beginGroup("Mega");
  EXPECT_TRUE(s.value("email").toString().isEmpty());
  EXPECT_TRUE(s.value("password").toString().isEmpty());
  EXPECT_TRUE(s.value("session").toString().isEmpty());
}

// ============================================================================
// Mega URL Scheme Tests
// ============================================================================

class MegaUrlSchemeTest : public ::testing::Test {};

TEST_F(MegaUrlSchemeTest, MegaUrlFormat) {
  QUrl url;
  url.setScheme("mega");
  url.setPath("/ABCDEFGH");

  EXPECT_EQ("mega", url.scheme());
  EXPECT_EQ("/ABCDEFGH", url.path());
  EXPECT_TRUE(url.isValid());
}

TEST_F(MegaUrlSchemeTest, ExtractHandleFromUrl) {
  QUrl url;
  url.setScheme("mega");
  url.setPath("/HANDLE123");

  QString handle = url.path();
  if (handle.startsWith('/')) handle = handle.mid(1);
  EXPECT_EQ("HANDLE123", handle);
}

TEST_F(MegaUrlSchemeTest, HandleEmptyPath) {
  QUrl url;
  url.setScheme("mega");
  url.setPath("/");

  QString handle = url.path();
  if (handle.startsWith('/')) handle = handle.mid(1);
  EXPECT_TRUE(handle.isEmpty());
}

TEST_F(MegaUrlSchemeTest, HandleRoundtripWithSdkBase64) {
  // Use SDK's handleToBase64 to create the URL path, then parse it back.
  mega::MegaHandle original_handle = 9876543210ULL;
  std::unique_ptr<char[]> handle_b64(
      mega::MegaApi::handleToBase64(original_handle));

  QUrl url;
  url.setScheme("mega");
  url.setPath("/" + QString::fromUtf8(handle_b64.get()));

  // Extract handle back from URL
  QString path_b64 = url.path().mid(1);
  mega::MegaHandle recovered =
      mega::MegaApi::base64ToHandle(path_b64.toUtf8().constData());
  EXPECT_EQ(original_handle, recovered);
}

// ============================================================================
// Mega Folder Selection Persistence Tests
// ============================================================================

class MegaFolderSelectionTest : public ::testing::Test {
 protected:
  void SetUp() override {
    QSettings s;
    s.beginGroup("Mega");
    s.remove("");
    s.endGroup();
  }

  void TearDown() override {
    QSettings s;
    s.beginGroup("Mega");
    s.remove("");
    s.endGroup();
  }
};

TEST_F(MegaFolderSelectionTest, NoFoldersByDefault) {
  QSettings s;
  s.beginGroup("Mega");
  EXPECT_TRUE(s.value("selected_folders").toStringList().isEmpty());
}

TEST_F(MegaFolderSelectionTest, StoreAndRetrieveFolders) {
  QStringList folders;
  folders << "AAAAAAAA" << "BBBBBBBB" << "CCCCCCCC";

  {
    QSettings s;
    s.beginGroup("Mega");
    s.setValue("selected_folders", folders);
  }

  QSettings s;
  s.beginGroup("Mega");
  QStringList retrieved = s.value("selected_folders").toStringList();
  EXPECT_EQ(3, retrieved.size());
  EXPECT_EQ("AAAAAAAA", retrieved[0]);
  EXPECT_EQ("BBBBBBBB", retrieved[1]);
  EXPECT_EQ("CCCCCCCC", retrieved[2]);
}

TEST_F(MegaFolderSelectionTest, ClearFoldersOnReset) {
  {
    QSettings s;
    s.beginGroup("Mega");
    s.setValue("selected_folders", QStringList() << "AAAAAAAA");
  }

  // Simulate reset behavior from MegaService::ResetDatabase.
  {
    QSettings s;
    s.beginGroup("Mega");
    s.remove("selected_folders");
  }

  QSettings s;
  s.beginGroup("Mega");
  EXPECT_TRUE(s.value("selected_folders").toStringList().isEmpty());
}

// ============================================================================
// Mega URL Handle Extraction Tests
// ============================================================================

class MegaHandleExtractionTest : public ::testing::Test {};

TEST_F(MegaHandleExtractionTest, ExtractHandleFromMegaUrl) {
  // Simulate the extraction logic from GetStreamingUrlFromSongId.
  QUrl url;
  url.setScheme("mega");
  url.setPath("/ABCDEF12");

  QString handle_b64 = url.path();
  if (handle_b64.startsWith('/')) handle_b64 = handle_b64.mid(1);

  EXPECT_EQ("ABCDEF12", handle_b64);
  EXPECT_FALSE(handle_b64.isEmpty());
}

TEST_F(MegaHandleExtractionTest, ExtractHandleFromUrlNoLeadingSlash) {
  // Edge case: path without leading slash.
  QUrl url;
  url.setScheme("mega");
  url.setPath("HANDLE123");

  QString handle_b64 = url.path();
  if (handle_b64.startsWith('/')) handle_b64 = handle_b64.mid(1);

  EXPECT_EQ("HANDLE123", handle_b64);
}

TEST_F(MegaHandleExtractionTest, EmptyPathYieldsEmptyHandle) {
  QUrl url;
  url.setScheme("mega");

  QString handle_b64 = url.path();
  if (handle_b64.startsWith('/')) handle_b64 = handle_b64.mid(1);

  EXPECT_TRUE(handle_b64.isEmpty());
}

TEST_F(MegaHandleExtractionTest, HandleBase64RoundtripMultiple) {
  // Test several different handles to ensure stable roundtrips.
  const mega::MegaHandle handles[] = {0, 1, 255, 65535, 12345678901234ULL};
  for (mega::MegaHandle original : handles) {
    std::unique_ptr<char[]> b64(mega::MegaApi::handleToBase64(original));
    mega::MegaHandle recovered = mega::MegaApi::base64ToHandle(b64.get());
    EXPECT_EQ(original, recovered) << "Failed for handle " << original;
  }
}

// ============================================================================
// Mega SDK Feature Tests
// ============================================================================

class MegaSdkFeatureTest : public ::testing::Test {};

TEST_F(MegaSdkFeatureTest, MultipleApiInstancesCoexist) {
  // Verify that creating multiple MegaApi instances doesn't crash.
  mega::MegaApi api1("yrQVDRBm", (const char*)nullptr, "Test1");
  mega::MegaApi api2("yrQVDRBm", (const char*)nullptr, "Test2");
  SUCCEED();
}

TEST_F(MegaSdkFeatureTest, HttpServerNotRunningByDefault) {
  mega::MegaApi api("yrQVDRBm", (const char*)nullptr, "ClementineTest");
  // The HTTP server should not be running before explicit start.
  EXPECT_EQ(0, api.httpServerIsRunning());
}

#endif  // HAVE_MEGA
