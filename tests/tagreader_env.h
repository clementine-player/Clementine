/* This file is part of Clementine.

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

#ifndef TAGREADER_ENV_H
#define TAGREADER_ENV_H

#include <gtest/gtest.h>

#include <QMetaObject>
#include <QThread>

#include "core/tagreaderclient.h"

// Real clementine-tagreader workers behind TagReaderClient::Instance(), for
// tests of code that reads tags through it. A test file that needs them
// registers this at namespace scope:
//
//   ::testing::Environment* const kTagReader =
//       ::testing::AddGlobalTestEnvironment(new TagReaderEnvironment);
//
// and is added with add_tagreader_test_file() in CMakeLists.txt, which builds
// the workers and tells the test where they are.
//
// The client gets a thread of its own, because its blocking calls, which the
// tests make, wait for replies that arrive on the client's thread.
class TagReaderEnvironment : public ::testing::Environment {
 public:
  void SetUp() {
    // The workers are found on $PATH.
    qputenv("PATH", QByteArray(TAGREADER_DIR) + ":" + qgetenv("PATH"));
    thread_ = new QThread;
    thread_->start();
    client_ = new TagReaderClient;
    client_->moveToThread(thread_);
    client_->Start();
  }

  void TearDown() {
    // Deleted on its own thread, which stops the workers.
    TagReaderClient* client = client_;
    QMetaObject::invokeMethod(
        client, [client]() { delete client; }, Qt::BlockingQueuedConnection);
    thread_->quit();
    thread_->wait();
    delete thread_;
  }

 private:
  QThread* thread_ = nullptr;
  TagReaderClient* client_ = nullptr;
};

#endif  // TAGREADER_ENV_H
