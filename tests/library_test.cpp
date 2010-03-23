#include "test_utils.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "library.h"
#include "backgroundthread.h"
#include "mock_backgroundthread.h"
#include "mock_librarybackend.h"

#include <QtDebug>
#include <QThread>
#include <QSignalSpy>
#include <QSortFilterProxyModel>

using ::testing::_;
using ::testing::Return;
using ::testing::StrictMock;

void PrintTo(const ::QString& str, std::ostream& os) {
  os << str.toStdString();
}

namespace {

class LibraryTest : public ::testing::Test {
 protected:
  void SetUp() {
    library_ = new StrictMock<Library>(
        static_cast<EngineBase*>(NULL), static_cast<QObject*>(NULL));
    library_->set_backend_factory(
        new FakeBackgroundThreadFactory<LibraryBackendInterface, MockLibraryBackend>);
    library_->set_watcher_factory(
        new FakeBackgroundThreadFactory<LibraryWatcher, LibraryWatcher>);

    library_->Init();

    backend_ = static_cast<MockLibraryBackend*>(library_->GetBackend().get());

    library_sorted_ = new QSortFilterProxyModel;
    library_sorted_->setSourceModel(library_);
    library_sorted_->setSortRole(Library::Role_SortText);
    library_sorted_->setDynamicSortFilter(true);
    library_sorted_->sort(0);
  }

  void TearDown() {
    EXPECT_CALL(*backend_, Die());
    delete library_;
    delete library_sorted_;
  }

  Library* library_;
  MockLibraryBackend* backend_;
  QSortFilterProxyModel* library_sorted_;
};

TEST_F(LibraryTest, Initialisation) {
  backend_->ExpectSetup();
  library_->StartThreads();

  EXPECT_EQ(0, library_->rowCount(QModelIndex()));
}

TEST_F(LibraryTest, WithInitialCompilations) {
  backend_->ExpectSetup(true);
  library_->StartThreads();

  ASSERT_EQ(1, library_->rowCount(QModelIndex()));

  QModelIndex va_index = library_->index(0, 0, QModelIndex());
  EXPECT_EQ("Various Artists", va_index.data().toString());
  EXPECT_TRUE(library_->hasChildren(va_index));
}

TEST_F(LibraryTest, WithInitialArtists) {
  backend_->ExpectSetup(false, QStringList() << "Artist 1" << "Artist 2" << "Foo");
  library_->StartThreads();

  ASSERT_EQ(5, library_sorted_->rowCount(QModelIndex()));
  EXPECT_EQ("a", library_sorted_->index(0, 0, QModelIndex()).data().toString());
  EXPECT_EQ("Artist 1", library_sorted_->index(1, 0, QModelIndex()).data().toString());
  EXPECT_EQ("Artist 2", library_sorted_->index(2, 0, QModelIndex()).data().toString());
  EXPECT_EQ("f", library_sorted_->index(3, 0, QModelIndex()).data().toString());
  EXPECT_EQ("Foo", library_sorted_->index(4, 0, QModelIndex()).data().toString());
}

TEST_F(LibraryTest, CompilationAlbums) {
  backend_->ExpectSetup(true);
  library_->StartThreads();

  QModelIndex va_index = library_->index(0, 0, QModelIndex());

  LibraryBackendInterface::AlbumList albums;
  albums << LibraryBackendInterface::Album("Artist", "Album", "", "");
  EXPECT_CALL(*backend_, GetCompilationAlbums(_))
      .WillOnce(Return(albums));

  ASSERT_EQ(library_->rowCount(va_index), 1);
  QModelIndex album_index = library_->index(0, 0, va_index);
  EXPECT_EQ(library_->data(album_index).toString(), "Album");
  EXPECT_TRUE(library_->hasChildren(album_index));
}


} // namespace
