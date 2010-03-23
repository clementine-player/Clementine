#include "test_utils.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "library.h"
#include "backgroundthread.h"
#include "mock_backgroundthread.h"
#include "mock_librarybackend.h"

#include <boost/scoped_ptr.hpp>

#include <QtDebug>
#include <QThread>
#include <QSignalSpy>

using ::testing::_;
using ::testing::Return;

class LibraryTest : public ::testing::Test {
 protected:
  virtual void SetUp() {
    library_.reset(new Library(NULL, NULL));
    library_->set_backend_factory(
        new FakeBackgroundThreadFactory<LibraryBackendInterface, MockLibraryBackend>);
    library_->set_watcher_factory(
        new FakeBackgroundThreadFactory<LibraryWatcher, LibraryWatcher>);

    library_->Init();

    backend_ = static_cast<MockLibraryBackend*>(library_->GetBackend().get());
  }

  boost::scoped_ptr<Library> library_;
  MockLibraryBackend* backend_;
};

TEST_F(LibraryTest, TestInitialisation) {
  EXPECT_CALL(*backend_, LoadDirectoriesAsync());
  EXPECT_CALL(*backend_, UpdateTotalSongCountAsync());
  EXPECT_CALL(*backend_, HasCompilations(_))
      .WillOnce(Return(false));
  EXPECT_CALL(*backend_, GetAllArtists(_))
      .WillOnce(Return(QStringList()));

  library_->StartThreads();
}
