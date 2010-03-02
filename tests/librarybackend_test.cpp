#include "test_utils.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "librarybackend.h"
#include "song.h"

#include <boost/scoped_ptr.hpp>

#include <QtDebug>
#include <QThread>

using ::testing::_;
using ::testing::AtMost;
using ::testing::Invoke;
using ::testing::Return;

void PrintTo(const ::QString& str, std::ostream& os) {
  os << str.toStdString();
}

class LibraryBackendTest : public ::testing::Test {
 protected:
  virtual void SetUp() {
    backend_.reset(new LibraryBackend(NULL, ":memory:"));

    connection_name_ = "thread_" + QString::number(
        reinterpret_cast<quint64>(QThread::currentThread()));
    database_ = QSqlDatabase::database(connection_name_);
  }

  void TearDown() {
    // Make sure Qt does not re-use the connection.
    database_ = QSqlDatabase();
    QSqlDatabase::removeDatabase(connection_name_);
  }

  Song MakeDummySong(int directory_id) {
    // Returns a valid song with all the required fields set
    Song ret;
    ret.set_directory_id(directory_id);
    ret.set_filename("foo.mp3");
    ret.set_mtime(0);
    ret.set_ctime(0);
    ret.set_filesize(0);
    return ret;
  }

  boost::scoped_ptr<LibraryBackend> backend_;
  QString connection_name_;
  QSqlDatabase database_;
};

TEST_F(LibraryBackendTest, DatabaseInitialises) {
  // Check that these tables exist
  QStringList tables = database_.tables();
  EXPECT_TRUE(tables.contains("songs"));
  EXPECT_TRUE(tables.contains("directories"));
  ASSERT_TRUE(tables.contains("schema_version"));

  // Check the schema version is correct
  QSqlQuery q("SELECT version FROM schema_version", database_);
  ASSERT_TRUE(q.exec());
  ASSERT_TRUE(q.next());
  EXPECT_EQ(2, q.value(0).toInt());
  EXPECT_FALSE(q.next());
}

TEST_F(LibraryBackendTest, EmptyDatabase) {
  // Check the database is empty to start with
  QStringList artists = backend_->GetAllArtists();
  EXPECT_TRUE(artists.isEmpty());

  LibraryBackend::AlbumList albums = backend_->GetAllAlbums();
  EXPECT_TRUE(albums.isEmpty());
}

TEST_F(LibraryBackendTest, AddSong) {
  // Add a directory
  backend_->AddDirectory("/test");

  // Add the song
  Song s = MakeDummySong(1);
  s.set_title("Foo");
  s.set_artist("Bar");
  s.set_album("Meep");
  backend_->AddOrUpdateSongs(SongList() << s);

  // Check the artist
  QStringList artists = backend_->GetAllArtists();
  ASSERT_EQ(1, artists.size());
  EXPECT_EQ(s.artist(), artists[0]);

  // Check the various album getters
  LibraryBackend::AlbumList albums = backend_->GetAllAlbums();
  ASSERT_EQ(1, albums.size());
  EXPECT_EQ(s.album(), albums[0].album_name);
  EXPECT_EQ(s.artist(), albums[0].artist);

  albums = backend_->GetAlbumsByArtist("Bar");
  ASSERT_EQ(1, albums.size());
  EXPECT_EQ(s.album(), albums[0].album_name);
  EXPECT_EQ(s.artist(), albums[0].artist);

  LibraryBackend::Album album = backend_->GetAlbumArt("Bar", "Meep");
  EXPECT_EQ(s.album(), album.album_name);
  EXPECT_EQ(s.artist(), album.artist);

  // Check we can get the song back
  SongList songs = backend_->GetSongs("Bar", "Meep");
  ASSERT_EQ(1, songs.size());
  EXPECT_EQ(s.album(), songs[0].album());
  EXPECT_EQ(s.artist(), songs[0].artist());
  EXPECT_EQ(s.title(), songs[0].title());
  EXPECT_EQ(1, songs[0].id());

  // Check we can get the song by ID
  Song song2 = backend_->GetSongById(1);
  EXPECT_EQ(s.album(), song2.album());
  EXPECT_EQ(s.artist(), song2.artist()); // This is an error - song2.artist() should obviously be "Blur"
  EXPECT_EQ(s.title(), song2.title());
  EXPECT_EQ(1, song2.id());
}

TEST_F(LibraryBackendTest, AddSongWithoutFilename) {
}

TEST_F(LibraryBackendTest, GetAlbumArtNonExistent) {
}
