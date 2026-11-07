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

#include "core/database.h"

#include <QSqlQuery>
#include <QTemporaryDir>
#include <QVariant>
#include <memory>

#include "core/song.h"
#include "core/timeconstants.h"
#include "gtest/gtest.h"
#include "library/library.h"
#include "library/librarybackend.h"
#include "library/libraryquery.h"
#include "test_utils.h"

namespace {

QString TableSql(const QSqlDatabase& db, const QString& table) {
  QSqlQuery q(db);
  q.prepare("SELECT sql FROM sqlite_master WHERE name = :name");
  q.bindValue(":name", table);
  if (!q.exec() || !q.next()) return QString();
  return q.value(0).toString();
}

int SchemaVersion(const QSqlDatabase& db) {
  QSqlQuery q("SELECT version FROM schema_version", db);
  if (!q.next()) return -1;
  return q.value(0).toInt();
}

Song MakeSong(const QString& title, const QString& artist,
              const QString& album) {
  Song song;
  song.Init(title, artist, album, 180 * kNsecPerSec);
  song.set_directory_id(1);
  song.set_url(QUrl::fromLocalFile("/music/" + title + ".flac"));
  song.set_mtime(1);
  song.set_ctime(1);
  song.set_filesize(1000);
  song.set_filetype(Song::Type_Flac);
  return song;
}

// How many library songs the library filter box would show for filter, or -1
// if the query fails.
int CountMatches(LibraryBackend* library, const QString& filter) {
  QueryOptions options;
  options.set_filter(filter);
  LibraryQuery query(options);
  query.SetColumnSpec("%songs_table.ROWID");
  if (!library->ExecQuery(&query)) return -1;
  int count = 0;
  while (query.Next()) ++count;
  return count;
}

}  // namespace

class DatabaseTest : public ::testing::Test {
 protected:
  void SetUp() override {
    database_.reset(new MemoryDatabase(nullptr));
    library_.reset(new LibraryBackend);
    library_->Init(database_.get(), Library::kSongsTable, Library::kDirsTable,
                   Library::kSubdirsTable, Library::kFtsTable);
    library_->AddDirectory("/music");
  }

  std::unique_ptr<Database> database_;
  std::unique_ptr<LibraryBackend> library_;
};

TEST_F(DatabaseTest, DatabaseInitialises) {
  QSqlDatabase db = database_->Connect();
  const QStringList tables = db.tables();
  EXPECT_TRUE(tables.contains("songs"));
  EXPECT_TRUE(tables.contains("directories"));
  EXPECT_TRUE(tables.contains("subdirectories"));
  EXPECT_TRUE(tables.contains("playlists"));
  EXPECT_TRUE(tables.contains("playlist_items"));
  EXPECT_EQ(Database::kSchemaVersion, SchemaVersion(db));
}

TEST_F(DatabaseTest, SearchTablesAreFts5) {
  QSqlDatabase db = database_->Connect();
  QSqlQuery q(
      "SELECT name FROM sqlite_master WHERE type = 'table'"
      " AND sql LIKE 'CREATE VIRTUAL TABLE%'",
      db);
  QStringList fts_tables;
  while (q.next()) {
    const QString name = q.value(0).toString();
    fts_tables << name;
    EXPECT_TRUE(TableSql(db, name).contains("fts5", Qt::CaseInsensitive))
        << name.toStdString();
  }
  EXPECT_TRUE(fts_tables.contains("songs_fts"));
  EXPECT_TRUE(fts_tables.contains("playlist_items_fts"));
}

TEST_F(DatabaseTest, SearchFindsPrefixesIgnoringCaseAndAccents) {
  library_->AddOrUpdateSongs(
      SongList() << MakeSong("Crazy in Love", "Beyoncé", "Dangerously")
                 << MakeSong("Sabotage", "Beastie Boys", "Ill"));

  EXPECT_EQ(1, CountMatches(library_.get(), "beyonce"));
  EXPECT_EQ(1, CountMatches(library_.get(), "BEYON"));
  EXPECT_EQ(1, CountMatches(library_.get(), "Beyoncé"));
  EXPECT_EQ(1, CountMatches(library_.get(), "crazy love"));
  EXPECT_EQ(2, CountMatches(library_.get(), "b"));
  EXPECT_EQ(0, CountMatches(library_.get(), "beyonce sabotage"));
}

TEST_F(DatabaseTest, SearchAcceptsPunctuation) {
  library_->AddOrUpdateSongs(
      SongList() << MakeSong("Back in Black", "AC/DC", "Back in Black")
                 << MakeSong("Don't Stop Me Now", "Queen", "Jazz")
                 << MakeSong("Sabotage", "Beastie Boys", "Ill"));

  EXPECT_EQ(1, CountMatches(library_.get(), "AC/DC"));
  EXPECT_EQ(1, CountMatches(library_.get(), "ac/d"));
  EXPECT_EQ(1, CountMatches(library_.get(), "don't"));
  EXPECT_EQ(0, CountMatches(library_.get(), "don\"t"));
  EXPECT_EQ(1, CountMatches(library_.get(), "(queen)"));
  EXPECT_EQ(0, CountMatches(library_.get(), "AND"));
  EXPECT_EQ(3, CountMatches(library_.get(), "-"));
}

TEST_F(DatabaseTest, SearchFiltersByColumn) {
  library_->AddOrUpdateSongs(SongList()
                             << MakeSong("Queen", "Patti Smith", "Wave")
                             << MakeSong("Bicycle Race", "Queen", "Jazz"));

  EXPECT_EQ(2, CountMatches(library_.get(), "queen"));
  EXPECT_EQ(1, CountMatches(library_.get(), "artist:queen"));
  EXPECT_EQ(1, CountMatches(library_.get(), "title:queen"));
  EXPECT_EQ(1, CountMatches(library_.get(), "artist:patti smith"));
  EXPECT_EQ(2, CountMatches(library_.get(), "artist:"));
}

TEST_F(DatabaseTest, SearchFollowsUpdatesAndDeletes) {
  Song song = MakeSong("Song 2", "Blur", "Blur");
  library_->AddOrUpdateSongs(SongList() << song);
  ASSERT_EQ(1, CountMatches(library_.get(), "blur"));

  song = library_->GetSongByUrl(song.url());
  ASSERT_TRUE(song.is_valid());
  song.set_artist("Gorillaz");
  song.set_mtime(2);
  library_->AddOrUpdateSongs(SongList() << song);
  EXPECT_EQ(1, CountMatches(library_.get(), "gorillaz"));
  EXPECT_EQ(1, CountMatches(library_.get(), "blur"));  // Still the album.
  EXPECT_EQ(0, CountMatches(library_.get(), "artist:blur"));

  library_->DeleteSongs(SongList() << song);
  EXPECT_EQ(0, CountMatches(library_.get(), "gorillaz"));
}

// A database from before schema 55 has FTS3 search tables using the
// "unicode" tokenizer. Opening it should replace them with FTS5 tables
// holding the same songs.
TEST(DatabaseMigrationTest, RebuildsFts3TablesAsFts5) {
  QTemporaryDir dir;
  ASSERT_TRUE(dir.isValid());
  const QString filename = dir.filePath("clementine.db");

  {
    Database database(nullptr, nullptr, filename);
    QSqlDatabase db = database.Connect();
    LibraryBackend library;
    library.Init(&database, Library::kSongsTable, Library::kDirsTable,
                 Library::kSubdirsTable, Library::kFtsTable);
    library.AddDirectory("/music");
    library.AddOrUpdateSongs(
        SongList() << MakeSong("Crazy in Love", "Beyoncé", "Dangerously"));
    QSqlQuery("DROP TABLE songs_fts", db);
    QSqlQuery create(db);
    ASSERT_TRUE(create.exec(
        "CREATE VIRTUAL TABLE songs_fts USING fts3("
        " ftstitle, ftsalbum, ftsartist, ftsalbumartist, ftscomposer,"
        " ftsperformer, ftsgrouping, ftsgenre, ftscomment, ftsyear,"
        " tokenize=unicode)"))
        << create.lastError().text().toStdString();
    QSqlQuery insert(db);
    ASSERT_TRUE(
        insert.exec("INSERT INTO songs_fts (ROWID, ftstitle, ftsartist)"
                    " SELECT ROWID, title, artist FROM songs"))
        << insert.lastError().text().toStdString();
    // And a device's table, which is named differently.
    QSqlQuery("CREATE TABLE device_7_songs AS SELECT * FROM songs", db);
    QSqlQuery device(db);
    ASSERT_TRUE(device.exec(
        "CREATE VIRTUAL TABLE device_7_fts USING fts3("
        " ftstitle, ftsalbum, ftsartist, ftsalbumartist, ftscomposer,"
        " ftsperformer, ftsgrouping, ftsgenre, ftscomment, ftsyear,"
        " tokenize=unicode)"));
    QSqlQuery("UPDATE schema_version SET version = 54", db);
  }
  for (const QString& name : QSqlDatabase::connectionNames()) {
    QSqlDatabase::removeDatabase(name);
  }

  Database database(nullptr, nullptr, filename);
  QSqlDatabase db = database.Connect();
  EXPECT_EQ(55, SchemaVersion(db));
  EXPECT_TRUE(TableSql(db, "songs_fts").contains("fts5"));
  EXPECT_TRUE(TableSql(db, "device_7_fts").contains("fts5"));
  EXPECT_FALSE(db.tables().contains("songs_fts_segdir"));

  LibraryBackend library;
  library.Init(&database, Library::kSongsTable, Library::kDirsTable,
               Library::kSubdirsTable, Library::kFtsTable);
  EXPECT_EQ(1, CountMatches(&library, "beyonce"));
  EXPECT_EQ(1, CountMatches(&library, "dangerous"));

  QSqlQuery device_search(
      "SELECT count(*) FROM device_7_fts WHERE device_7_fts MATCH '\"crazy\"*'",
      db);
  ASSERT_TRUE(device_search.next());
  EXPECT_EQ(1, device_search.value(0).toInt());
}
