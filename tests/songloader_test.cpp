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

#include "test_utils.h"
#include "gmock/gmock-matchers.h"
#include "gtest/gtest.h"
#include "mock_librarybackend.h"

#include "core/songloader.h"
#include "engines/gstengine.h"

#include <QBuffer>
#include <QDir>
#include <QEventLoop>
#include <QSignalSpy>
#include <QtDebug>

#include <boost/scoped_ptr.hpp>
#include <cstdlib>

using ::testing::_;
using ::testing::Return;

class SongLoaderTest : public ::testing::Test {
public:
  static void SetUpTestCase() {
    sGstEngine = new GstEngine;
    ASSERT_TRUE(sGstEngine->Init());
    sGstEngine->EnsureInitialised();
  }

  static void TearDownTestCase() {
    delete sGstEngine;
    sGstEngine = NULL;
  }

protected:
  void SetUp() {
    library_.reset(new MockLibraryBackend);
    loader_.reset(new SongLoader(library_.get(), nullptr));
    loader_->set_timeout(20000);

    // the thing we return is not really important
    EXPECT_CALL(*library_.get(), GetSongByUrl(_, _)).WillRepeatedly(Return(Song()));
  }

  void LoadLocalDirectory(const QString& dir);

  static const char* kRemoteUrl;
  static GstEngine* sGstEngine;

  boost::scoped_ptr<SongLoader> loader_;
  boost::scoped_ptr<MockLibraryBackend> library_;
};

const char* SongLoaderTest::kRemoteUrl = "http://remotetestdata.clementine-player.org";
GstEngine* SongLoaderTest::sGstEngine = NULL;

TEST_F(SongLoaderTest, LoadLocalMp3) {
  TemporaryResource file(":/testdata/beep.mp3");
  SongLoader::Result ret = loader_->Load(QUrl::fromLocalFile(file.fileName()));

  ASSERT_EQ(SongLoader::Success, ret);
  ASSERT_EQ(1, loader_->songs().count());
  EXPECT_TRUE(loader_->songs()[0].is_valid());
  EXPECT_EQ("Beep mp3", loader_->songs()[0].title());
}

TEST_F(SongLoaderTest, LoadLocalFileQueryExecutesButEmpty) {
  EXPECT_CALL(*library_.get(), ExecQuery(_)).WillOnce(Return(true));
  TemporaryResource file(":/testdata/beep.mp3");
  SongLoader::Result ret = loader_->Load(QUrl::fromLocalFile(file.fileName()));

  ASSERT_EQ(SongLoader::Success, ret);
  ASSERT_EQ(1, loader_->songs().count());
  EXPECT_TRUE(loader_->songs()[0].is_valid());
  EXPECT_EQ("Beep mp3", loader_->songs()[0].title());
}

TEST_F(SongLoaderTest, LoadLocalPls) {
  TemporaryResource file(":/testdata/pls_one.pls");
  SongLoader::Result ret = loader_->Load(QUrl::fromLocalFile(file.fileName()));

  ASSERT_EQ(SongLoader::WillLoadAsync, ret);
  QSignalSpy spy(loader_.get(), SIGNAL(LoadFinished(bool)));

  // Start an event loop to wait for gstreamer to do its thing
  QEventLoop loop;
  QObject::connect(loader_.get(), SIGNAL(LoadFinished(bool)),
                   &loop, SLOT(quit()));
  loop.exec(QEventLoop::ExcludeUserInputEvents);

  // Check the signal was emitted with Success
  ASSERT_EQ(1, spy.count());
  EXPECT_EQ(true, spy[0][0].toBool());

  // Check the song got loaded
  ASSERT_EQ(1, loader_->songs().count());
  EXPECT_EQ("Title", loader_->songs()[0].title());
  EXPECT_EQ(123 * kNsecPerSec, loader_->songs()[0].length_nanosec());
}

TEST_F(SongLoaderTest, LoadLocalM3U) {
  TemporaryResource file(":/testdata/test.m3u");
  SongLoader::Result ret = loader_->Load(QUrl::fromLocalFile(file.fileName()));

  QSignalSpy spy(loader_.get(), SIGNAL(LoadFinished(bool)));

  QEventLoop loop;
  QObject::connect(loader_.get(), SIGNAL(LoadFinished(bool)),
                   &loop, SLOT(quit()));
  loop.exec(QEventLoop::ExcludeUserInputEvents);

  // Check the signal was emitted with Success
  ASSERT_EQ(1, spy.count());
  EXPECT_EQ(true, spy[0][0].toBool());

  ASSERT_EQ(SongLoader::WillLoadAsync, ret);
  ASSERT_EQ(239, loader_->songs().count());
}

TEST_F(SongLoaderTest, LoadLocalXSPF) {
  TemporaryResource file(":/testdata/test.xspf");
  SongLoader::Result ret = loader_->Load(QUrl::fromLocalFile(file.fileName()));

  QSignalSpy spy(loader_.get(), SIGNAL(LoadFinished(bool)));

  QEventLoop loop;
  QObject::connect(loader_.get(), SIGNAL(LoadFinished(bool)),
                   &loop, SLOT(quit()));
  loop.exec(QEventLoop::ExcludeUserInputEvents);

  // Check the signal was emitted with Success
  ASSERT_EQ(1, spy.count());
  EXPECT_EQ(true, spy[0][0].toBool());

  ASSERT_EQ(SongLoader::WillLoadAsync, ret);
  ASSERT_EQ(1, loader_->songs().count());
  EXPECT_EQ("Foo", loader_->songs()[0].title());
}

TEST_F(SongLoaderTest, LoadLocalASX) {
  TemporaryResource file(":/testdata/test.asx");
  SongLoader::Result ret = loader_->Load(QUrl::fromLocalFile(file.fileName()));

  QSignalSpy spy(loader_.get(), SIGNAL(LoadFinished(bool)));

  QEventLoop loop;
  QObject::connect(loader_.get(), SIGNAL(LoadFinished(bool)),
                   &loop, SLOT(quit()));
  loop.exec(QEventLoop::ExcludeUserInputEvents);

  // Check the signal was emitted with Success
  ASSERT_EQ(1, spy.count());
  EXPECT_EQ(true, spy[0][0].toBool());

  ASSERT_EQ(SongLoader::WillLoadAsync, ret);
  ASSERT_EQ(1, loader_->songs().count());
  EXPECT_EQ("Foo", loader_->songs()[0].title());
}

TEST_F(SongLoaderTest, LoadRemoteMp3) {
  SongLoader::Result ret = loader_->Load(QString(kRemoteUrl) + "/beep.mp3");
  ASSERT_EQ(SongLoader::WillLoadAsync, ret);

  QSignalSpy spy(loader_.get(), SIGNAL(LoadFinished(bool)));

  // Start an event loop to wait for gstreamer to do its thing
  QEventLoop loop;
  QObject::connect(loader_.get(), SIGNAL(LoadFinished(bool)),
                   &loop, SLOT(quit()));
  loop.exec(QEventLoop::ExcludeUserInputEvents);

  // Check the signal was emitted with Success
  ASSERT_EQ(1, spy.count());
  EXPECT_EQ(true, spy[0][0].toBool());

  // Check the song got loaded
  ASSERT_EQ(1, loader_->songs().count());
  EXPECT_EQ(QUrl(QString(kRemoteUrl) + "/beep.mp3"), loader_->songs()[0].url());
}

TEST_F(SongLoaderTest, LoadRemote404) {
  SongLoader::Result ret = loader_->Load(QString(kRemoteUrl) + "/404.mp3");
  ASSERT_EQ(SongLoader::WillLoadAsync, ret);

  QSignalSpy spy(loader_.get(), SIGNAL(LoadFinished(bool)));

  // Start an event loop to wait for gstreamer to do its thing
  QEventLoop loop;
  QObject::connect(loader_.get(), SIGNAL(LoadFinished(bool)),
                   &loop, SLOT(quit()));
  loop.exec(QEventLoop::ExcludeUserInputEvents);

  // Check the signal was emitted with Error
  ASSERT_EQ(1, spy.count());
  EXPECT_EQ(false, spy[0][0].toBool());
}

TEST_F(SongLoaderTest, LoadRemotePls) {
  SongLoader::Result ret = loader_->Load(QString(kRemoteUrl) + "/pls_somafm.pls");
  ASSERT_EQ(SongLoader::WillLoadAsync, ret);

  QSignalSpy spy(loader_.get(), SIGNAL(LoadFinished(bool)));

  // Start an event loop to wait for gstreamer to do its thing
  QEventLoop loop;
  QObject::connect(loader_.get(), SIGNAL(LoadFinished(bool)),
                   &loop, SLOT(quit()));
  loop.exec(QEventLoop::ExcludeUserInputEvents);

  // Check the signal was emitted with Success
  ASSERT_EQ(1, spy.count());
  EXPECT_EQ(true, spy[0][0].toBool());

  // Check some metadata
  ASSERT_EQ(4, loader_->songs().count());
  EXPECT_EQ("SomaFM: Groove Salad (#3 128k mp3): A nicely chilled plate of ambient beats and grooves.",
            loader_->songs()[2].title());
  EXPECT_EQ(QUrl("http://ice.somafm.com/groovesalad"), loader_->songs()[3].url());
}

TEST_F(SongLoaderTest, LoadRemotePlainText) {
  SongLoader::Result ret = loader_->Load(QString(kRemoteUrl) + "/notaplaylist.txt");
  ASSERT_EQ(SongLoader::WillLoadAsync, ret);

  QSignalSpy spy(loader_.get(), SIGNAL(LoadFinished(bool)));

  // Start an event loop to wait for gstreamer to do its thing
  QEventLoop loop;
  QObject::connect(loader_.get(), SIGNAL(LoadFinished(bool)),
                   &loop, SLOT(quit()));
  loop.exec(QEventLoop::ExcludeUserInputEvents);

  // Check the signal was emitted with Error
  ASSERT_EQ(1, spy.count());
  EXPECT_EQ(false, spy[0][0].toBool());
}

TEST_F(SongLoaderTest, LoadRemotePlainM3U) {
  SongLoader::Result ret = loader_->Load(QString(kRemoteUrl) + "/plainm3u.m3u");
  ASSERT_EQ(SongLoader::WillLoadAsync, ret);

  QSignalSpy spy(loader_.get(), SIGNAL(LoadFinished(bool)));

  // Start an event loop to wait for gstreamer to do its thing
  QEventLoop loop;
  QObject::connect(loader_.get(), SIGNAL(LoadFinished(bool)),
                   &loop, SLOT(quit()));
  loop.exec(QEventLoop::ExcludeUserInputEvents);

  // Check the signal was emitted with Success
  ASSERT_EQ(1, spy.count());
  EXPECT_EQ(true, spy[0][0].toBool());

  ASSERT_EQ(2, loader_->songs().count());
  EXPECT_EQ(QUrl("http://www.example.com/one.mp3"), loader_->songs()[0].url());
  EXPECT_EQ(QUrl("http://www.example.com/two.mp3"), loader_->songs()[1].url());
}

TEST_F(SongLoaderTest, LoadLocalDirectory) {
  // Make a directory and shove some files in it
  QString dir;
  {
    QTemporaryFile ffffuuuuuuuu;
    ffffuuuuuuuu.open();
    dir = ffffuuuuuuuu.fileName();
  }

  QDir d;
  ASSERT_TRUE(d.mkdir(dir));

  QFile resource(":/testdata/beep.mp3");
  resource.open(QIODevice::ReadOnly);
  QByteArray data(resource.readAll());

  // Write 3 MP3 files
  for (int i=0 ; i<3 ; ++i) {
    QFile mp3(QString("%1/%2.mp3").arg(dir).arg(i));
    mp3.open(QIODevice::WriteOnly);
    mp3.write(data);
  }

  // And one file that isn't an MP3
  QFile somethingelse(QString(dir) + "/somethingelse.foo");
  somethingelse.open(QIODevice::WriteOnly);
  somethingelse.write("I'm not an MP3!");
  somethingelse.close();

  // The actual test happens in another function so we can always clean up if
  // it asserts
  LoadLocalDirectory(dir);

  QFile::remove(QString(dir) + "/0.mp3");
  QFile::remove(QString(dir) + "/1.mp3");
  QFile::remove(QString(dir) + "/2.mp3");
  QFile::remove(QString(dir) + "/somethingelse.foo");

  d.rmdir(dir);
}

void SongLoaderTest::LoadLocalDirectory(const QString &filename) {
  // Load the directory
  SongLoader::Result ret = loader_->Load(QUrl::fromLocalFile(filename));
  ASSERT_EQ(SongLoader::WillLoadAsync, ret);

  QSignalSpy spy(loader_.get(), SIGNAL(LoadFinished(bool)));

  // Start an event loop to wait for it to read the directory
  QEventLoop loop;
  QObject::connect(loader_.get(), SIGNAL(LoadFinished(bool)),
                   &loop, SLOT(quit()));
  loop.exec(QEventLoop::ExcludeUserInputEvents);
  loader_.get()->EffectiveSongsLoad();

  // Check the signal was emitted with Success
  ASSERT_EQ(1, spy.count());
  EXPECT_EQ(true, spy[0][0].toBool());

  // Check it loaded three files
  ASSERT_EQ(3, loader_->songs().count());
  EXPECT_EQ("Beep mp3", loader_->songs()[2].title());
}
