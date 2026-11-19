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

#include "core/macfslistener.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMetaObject>
#include <QStringList>
#include <QTemporaryDir>
#include <QTest>
#include <new>

#include "gtest/gtest.h"

namespace {

class MacFSListenerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    ASSERT_TRUE(dir_.isValid());
    // FSEvents reports the real path, without /var's symlink to
    // /private/var.
    path_ = QFileInfo(dir_.path()).canonicalFilePath();
  }

  // Starts watching path_ now, rather than after the listener's delay.
  static void Watch(MacFSListener* listener, const QString& path) {
    listener->AddPath(path);
    QMetaObject::invokeMethod(listener, "UpdateStream");
  }

  void TouchFile(const QString& name) {
    QFile file(QDir(path_).filePath(name));
    ASSERT_TRUE(file.open(QIODevice::WriteOnly));
    file.write("x");
  }

  // Waits for the listener to report path_. The listener reports on its own
  // queue, so the paths come to this thread through the event loop.
  static bool WaitForChange(const QStringList& changed, const QString& path) {
    return QTest::qWaitFor([&]() { return changed.contains(path); }, 30000);
  }

  QTemporaryDir dir_;
  QString path_;
};

TEST_F(MacFSListenerTest, ReportsChanges) {
  MacFSListener listener;
  QStringList changed;
  QObject::connect(&listener, &MacFSListener::PathChanged, &listener,
                   [&changed](const QString& path) { changed << path; });
  Watch(&listener, path_);

  TouchFile("song.mp3");

  EXPECT_TRUE(WaitForChange(changed, path_));
}

// A filesystem device's listener goes when the device is removed, and the
// app's when it quits, while their paths can still change (CLEMENTINE-6).
TEST_F(MacFSListenerTest, DeletedListenerIsNotCalled) {
  // Another listener is made in the deleted one's memory, so a callback that
  // still comes to that address reaches a listener that reports it, rather
  // than freed memory that may or may not crash.
  alignas(MacFSListener) unsigned char storage[sizeof(MacFSListener)];
  MacFSListener* deleted = new (storage) MacFSListener;
  Watch(deleted, path_);
  deleted->~MacFSListener();

  MacFSListener* in_its_place = new (storage) MacFSListener;
  QStringList reached_deleted;
  QObject::connect(
      in_its_place, &MacFSListener::PathChanged, in_its_place,
      [&reached_deleted](const QString& path) { reached_deleted << path; });

  // A listener on the same path. FSEvents reports the change to both
  // streams, so once this one has it, the deleted one would have had it too.
  MacFSListener listener;
  QStringList changed;
  QObject::connect(&listener, &MacFSListener::PathChanged, &listener,
                   [&changed](const QString& path) { changed << path; });
  Watch(&listener, path_);

  TouchFile("song.mp3");

  EXPECT_TRUE(WaitForChange(changed, path_));
  // FSEvents doesn't promise to deliver the two streams' events together, so
  // give the deleted one's twice its latency to come in too. This only
  // decides whether a listener that is still called gets caught: with one
  // that isn't, the test passes however long this is.
  QTest::qWait(2000);
  EXPECT_TRUE(reached_deleted.isEmpty())
      << reached_deleted.join(", ").toStdString();

  in_its_place->~MacFSListener();
}

}  // namespace
