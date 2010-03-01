#include "gmock/gmock-printers.h"
#include "test_utils.h"
#include "gtest/gtest.h"
#include "mock_sqldriver.h"

#include "librarybackend.h"

#include <boost/scoped_ptr.hpp>

#include <QtDebug>
#include <QThread>

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;

using boost::scoped_ptr;

void PrintTo(const ::QString& str, std::ostream& os) {
  os << str.toStdString();
}

class LibraryBackendTest : public ::testing::Test {
 protected:
  LibraryBackendTest() {
  }

  virtual void SetUp() {
    driver_ = new MockSqlDriver;
    // DB connect calls.
    EXPECT_CALL(*driver_, open(_, _, _, _, _, _)).WillOnce(Return(true));
    EXPECT_CALL(*driver_, isOpen()).WillRepeatedly(Return(true));
    EXPECT_CALL(*driver_, hasFeature(_)).WillRepeatedly(Return(true));

    QStringList tables;
    tables << "Foo";
    EXPECT_CALL(*driver_, tables(QSql::Tables)).WillOnce(Return(tables));

    MockSqlResult* result = new MockSqlResult(driver_);
    EXPECT_CALL(*driver_, createResult()).
        WillOnce(Return(result));
    EXPECT_CALL(*result, reset(QString("SELECT version FROM schema_version"))).WillOnce(
        DoAll(
            Invoke(result, &MockSqlResult::hackSetActive),
            Return(true)));
    EXPECT_CALL(*result, fetch(1)).WillOnce(
        DoAll(
            Invoke(result, &MockSqlResult::setAt),
            Return(true)));
    EXPECT_CALL(*result, data(0)).WillOnce(Return(QVariant(2)));

    EXPECT_CALL(*driver_, close());

    backend_.reset(new LibraryBackend(NULL, driver_));
  }

  void TearDown() {
    // Make sure Qt does not re-use the connection.
    QSqlDatabase::removeDatabase("thread_" + QString::number(
        reinterpret_cast<quint64>(QThread::currentThread())));
  }

  MockSqlDriver* driver_;
  scoped_ptr<LibraryBackend> backend_;
};

MATCHER(Printer, "") { qDebug() << arg; return true; }

TEST_F(LibraryBackendTest, DatabaseInitialises) {

}

TEST_F(LibraryBackendTest, GetSongs) {
  const char* query =
      "SELECT ROWID, title, album, artist, albumartist, composer, "
      "track, disc, bpm, year, genre, comment, compilation, "
      "length, bitrate, samplerate, directory, filename, "
      "mtime, ctime, filesize, sampler, art_automatic, art_manual"
      " FROM songs "
      "WHERE (compilation = 0 AND sampler = 0) AND artist = ?"
      " AND album = ?";
  MockSqlResult* result = new MockSqlResult(driver_);
  EXPECT_CALL(*driver_, createResult()).
      WillOnce(Return(result));
  EXPECT_CALL(*result, reset(QString(query))).WillOnce(
      DoAll(
          Invoke(result, &MockSqlResult::hackSetActive),
          Return(true)));

  EXPECT_CALL(*result, bindValue(0, QVariant("foo"), _));
  EXPECT_CALL(*result, bindValue(1, QVariant("bar"), _));

  EXPECT_CALL(*result, exec()).WillOnce(Return(true));
  EXPECT_CALL(*result, fetch(1)).WillOnce(
      DoAll(
          Invoke(result, &MockSqlResult::setAt),
          Return(true)));
  EXPECT_CALL(*result, fetch(2)).WillOnce(Return(false));

  for (int i = 0; i < 24; ++i) {
    // Fill every column with 42.
    EXPECT_CALL(*result, data(i)).WillRepeatedly(
        Return(QVariant(42))).RetiresOnSaturation();
  }

  SongList songs = backend_->GetSongs("foo", "bar");
  ASSERT_EQ(1, songs.length());
  EXPECT_EQ(42, songs[0].length());
  EXPECT_EQ(42, songs[0].id());
}
