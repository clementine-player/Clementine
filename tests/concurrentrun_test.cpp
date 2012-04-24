#include "gtest/gtest.h"

#include <QtGlobal>
#include <QEventLoop>
#include <QFutureWatcher>
#include <QThreadPool>

#include "core/concurrentrun.h"
#include "test_utils.h"

int max(int i, int j) {
  return (i > j ? i : j);
}

TEST(ConcurrentRunTest, ConcurrentRunStartAndWait) {
  int i = 10;
  int j = 42;
  QThreadPool threadpool;
  QFuture<int> future = ConcurrentRun::Run<int, int, int>(&threadpool, &max, i, j);
  QFutureWatcher<int> watcher;
  watcher.setFuture(future);
  QEventLoop loop;
  QObject::connect(&watcher, SIGNAL(finished()), &loop, SLOT(quit()));
  loop.exec();
  EXPECT_EQ(42, watcher.result());
}

// TODO: add some more complex test cases? (e.g. with several CPU-consuming
// tasks launched in parallel, with/without threadpool's threads numbers
// decreased, etc.)
