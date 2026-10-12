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

#include "core/tagreaderclient.h"

#include <QFile>
#include <QSettings>
#include <QTemporaryDir>
#include <QTest>
#include <QThread>

#include "core/song.h"
#include "gtest/gtest.h"
#include "test_utils.h"

namespace {

const int kTimeoutMsec = 30000;

QTemporaryDir* sSettingsDir = nullptr;
QThread* sTagReaderThread = nullptr;
TagReaderClient* sTagReader = nullptr;

// Talks to real tag reader workers, on their own thread as in the app.
class TagReaderClientTest : public ::testing::Test {
 protected:
  static void SetUpTestCase() {
    sSettingsDir = new QTemporaryDir;
    QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope,
                       sSettingsDir->path());
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope,
                       sSettingsDir->path());

    // The workers are found on $PATH.
    qputenv("PATH", QByteArray(TAGREADER_DIR) + ":" + qgetenv("PATH"));
    sTagReaderThread = new QThread;
    sTagReaderThread->start();
    sTagReader = new TagReaderClient;
    sTagReader->moveToThread(sTagReaderThread);
    sTagReader->Start();
  }

  static void TearDownTestCase() {
    // Deleted on its own thread, which stops the workers.
    QMetaObject::invokeMethod(
        sTagReader, []() { delete sTagReader; }, Qt::BlockingQueuedConnection);
    sTagReaderThread->quit();
    sTagReaderThread->wait();
    delete sTagReaderThread;
    delete sSettingsDir;
  }

  // A copy of the test MP3 to write to.
  QString CopyOfBeep() {
    const QString path = dir_.path() + "/beep.mp3";
    QFile::remove(path);
    EXPECT_TRUE(QFile::copy(TEST_DATA_DIR "/beep.mp3", path));
    QFile(path).setPermissions(QFile::ReadOwner | QFile::WriteOwner);
    return path;
  }

  static Song ReadSong(const QString& path) {
    Song song;
    sTagReader->ReadFileBlocking(path, &song);
    return song;
  }

  static Song SongFor(const QString& path, const QString& title) {
    Song song = ReadSong(path);
    song.set_title(title);
    return song;
  }

  QTemporaryDir dir_;
};

TEST_F(TagReaderClientTest, SavingContinuesOnTheCallersThread) {
  const QString path = CopyOfBeep();
  QObject context;
  bool called = false;
  bool saved = false;
  QThread* thread = nullptr;

  sTagReader->SaveFile(path, SongFor(path, "Saved through a promise"))
      .then(&context, [&](bool result) {
        called = true;
        saved = result;
        thread = QThread::currentThread();
      });

  ASSERT_TRUE(QTest::qWaitFor([&called]() { return called; }, kTimeoutMsec));
  EXPECT_TRUE(saved);
  EXPECT_EQ(QThread::currentThread(), thread);
  EXPECT_EQ(QString("Saved through a promise"), ReadSong(path).title());
}

TEST_F(TagReaderClientTest, AFileThatCantBeWrittenIsntSaved) {
  QObject context;
  bool called = false;
  bool saved = true;

  const QString path = dir_.path() + "/missing/beep.mp3";
  Song song;
  song.set_url(QUrl::fromLocalFile(path));
  song.set_title("Nowhere");
  sTagReader->SaveFile(path, song).then(&context, [&](bool result) {
    called = true;
    saved = result;
  });

  ASSERT_TRUE(QTest::qWaitFor([&called]() { return called; }, kTimeoutMsec));
  EXPECT_FALSE(saved);
}

TEST_F(TagReaderClientTest, NothingContinuesForADeletedContext) {
  const QString path = CopyOfBeep();
  QObject* context = new QObject;
  bool called = false;

  QFuture<bool> future =
      sTagReader->SaveFile(path, SongFor(path, "Nobody's listening"));
  future.then(context, [&called](bool) { called = true; });
  delete context;

  ASSERT_TRUE(QTest::qWaitFor([&future]() { return future.isFinished(); },
                              kTimeoutMsec));
  QCoreApplication::processEvents();
  EXPECT_FALSE(called);
  // The save itself still happened.
  EXPECT_TRUE(future.result());
}

}  // namespace
