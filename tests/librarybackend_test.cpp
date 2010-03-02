#include "gmock/gmock-printers.h"
#include "test_utils.h"
#include "gtest/gtest.h"
#include "mock_sqldriver.h"

#include "librarybackend.h"

#include <boost/scoped_ptr.hpp>

#include <QtDebug>
#include <QThread>

using ::testing::_;
using ::testing::AtMost;
using ::testing::Invoke;
using ::testing::Return;

using boost::scoped_ptr;

void PrintTo(const ::QString& str, std::ostream& os) {
  os << str.toStdString();
}

class LibraryBackendTest : public ::testing::Test {
 protected:
  LibraryBackendTest()
      : current_result_(0) {
  }

  virtual void SetUp() {
    // Owned by QSqlDatabase.
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

  // Call this to mock a single query with one row of results.
  void ExpectQuery(const QString& query,
                   const QList<QVariant>& bind_values,
                   const QMap<QString, QVariant>& named_bind_values,
                   const QList<QVariant>& columns) {
    // Owned by QSqlDatabase.
    current_result_ = new MockSqlResult(driver_);
    EXPECT_CALL(*driver_, createResult()).
        WillOnce(Return(current_result_));

    // Query string is set.
    EXPECT_CALL(*current_result_, reset(QString(query))).WillOnce(
        DoAll(
            Invoke(current_result_, &MockSqlResult::hackSetActive),
            Return(true)));
    // Query is executed.
    EXPECT_CALL(*current_result_, exec()).WillOnce(Return(true));

    // Values are bound.
    for (int i = 0; i < bind_values.size(); ++i) {
      ExpectBind(i, bind_values[i]);
    }
    for (QMap<QString, QVariant>::const_iterator it = named_bind_values.begin();
         it != named_bind_values.end(); ++it) {
      ExpectBind(it.key(), it.value());
    }

    // One row is fetched.
    EXPECT_CALL(*current_result_, fetch(1)).WillOnce(
        DoAll(
            Invoke(current_result_, &MockSqlResult::setAt),
            Return(true)));
    // Tries to fetch second row but we say we do not have any more rows.
    // Can be called 0-1 times. This catches the case where a query is only expected
    // to have one row so QSqlQuery::next() is only called once.
    EXPECT_CALL(*current_result_, fetch(2)).Times(AtMost(1)).WillOnce(Return(false));

    // Expect data() calls for each column.
    ExpectData(columns);
  }

  void ExpectBind(int index, const QVariant& value) {
    EXPECT_CALL(*current_result_, bindValue(index, value, _));
  }

  void ExpectBind(const QString& bind, const QVariant& value) {
    EXPECT_CALL(*current_result_, bindValue(bind, value, _));
  }

  void ExpectData(const QList<QVariant>& columns) {
    for (int i = 0; i < columns.size(); ++i) {
      EXPECT_CALL(*current_result_, data(i)).WillRepeatedly(
          Return(columns[i])).RetiresOnSaturation();
    }
  }

  MockSqlDriver* driver_;
  scoped_ptr<LibraryBackend> backend_;

  MockSqlResult* current_result_;
};

TEST_F(LibraryBackendTest, DatabaseInitialises) {

}

TEST_F(LibraryBackendTest, GetSongsSuccessfully) {
  const char* query =
      "SELECT ROWID, title, album, artist, albumartist, composer, "
      "track, disc, bpm, year, genre, comment, compilation, "
      "length, bitrate, samplerate, directory, filename, "
      "mtime, ctime, filesize, sampler, art_automatic, art_manual"
      " FROM songs "
      "WHERE (compilation = 0 AND sampler = 0) AND artist = ?"
      " AND album = ?";
  QList<QVariant> bind_values;
  bind_values << "foo";
  bind_values << "bar";

  QList<QVariant> columns;
  for (int i = 0; i < 24; ++i) {
    // Fill every column with 42.
    columns << 42;
  }

  ExpectQuery(query, bind_values, QMap<QString, QVariant>(), columns);

  SongList songs = backend_->GetSongs("foo", "bar");
  ASSERT_EQ(1, songs.length());
  EXPECT_EQ(42, songs[0].length());
  EXPECT_EQ(42, songs[0].id());
}

TEST_F(LibraryBackendTest, GetSongByIdSuccessfully) {
  const char* query =
      "SELECT ROWID, title, album, artist, albumartist, composer, "
      "track, disc, bpm, year, genre, comment, compilation, "
      "length, bitrate, samplerate, directory, filename, "
      "mtime, ctime, filesize, sampler, art_automatic, art_manual"
      " FROM songs "
      "WHERE ROWID = :id";
  QMap<QString, QVariant> bind_values;
  bind_values[":id"] = 42;

  QList<QVariant> columns;
  for (int i = 0; i < 24; ++i) {
    // Fill every column with 42.
    columns << 42;
  }

  ExpectQuery(query, QList<QVariant>(), bind_values, columns);

  Song song = backend_->GetSongById(42);
  EXPECT_EQ(42, song.length());
  EXPECT_EQ(42, song.id());
}
