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

// Scans real directories with LibraryWatcher, reading tags with real
// clementine-tagreader workers, into an in-memory library.
//
// The app runs the watcher on its own thread and the TagReaderClient on the
// main one. Here it's the other way round: the watcher, backend and database
// stay on the test's thread, which keeps to one database connection (each
// connection to :memory: is a separate database), and the TagReaderClient gets
// a thread of its own. Either way the watcher's blocking reads never run on
// the TagReaderClient's thread.

#include "library/librarywatcher.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMap>
#include <QSettings>
#include <QTemporaryDir>
#include <QTest>
#include <QThread>
#include <chrono>
#include <filesystem>
#include <functional>
#include <memory>

#include "core/database.h"
#include "core/song.h"
#include "core/tagreaderclient.h"
#include "core/taskmanager.h"
#include "gtest/gtest.h"
#include "library/library.h"
#include "library/librarybackend.h"
#include "library/librarytagprefetcher.h"
#include "tagreader_env.h"
#include "test_utils.h"

namespace {

const int kTimeoutMsec = 30000;
// More files than the prefetcher keeps requests for, so its window refills.
const int kManyFiles = LibraryTagPrefetcher::kMaxInFlight + 8;

// Somewhere for settings and Clementine's config directory that isn't the
// user's.
QTemporaryDir* sSettingsDir = nullptr;
QThread* sTagReaderThread = nullptr;
TagReaderClient* sTagReader = nullptr;

// Runs a test's hook when the watcher asks for a directory's songs, which a
// scan does after listing the first subdirectory it looks in and before going
// through its files.
class HookedLibraryBackend : public LibraryBackend {
 public:
  SongList FindSongsInDirectory(int id) override {
    if (before_find_songs_) {
      // Once: the hook is the test's, not every scan's.
      std::function<void()> hook;
      hook.swap(before_find_songs_);
      hook();
    }
    return LibraryBackend::FindSongsInDirectory(id);
  }

  std::function<void()> before_find_songs_;
};

class LibraryWatcherTest : public ::testing::Test {
 protected:
  static void SetUpTestCase() {
    sSettingsDir = new QTemporaryDir;
    // In an INI file there rather than, on Windows, the registry, which
    // setPath() doesn't move and which this test has no name in.
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope,
                       sSettingsDir->path());
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope,
                       sSettingsDir->path());
    // Database creates Clementine's config directory.
    qputenv("XDG_CONFIG_HOME", QFile::encodeName(sSettingsDir->path()));
    {
      QSettings s;
      // Tests rescan when they choose, not when files change.
      s.setValue("LibraryWatcher/monitor", false);
      s.setValue("Player/max_numprocs_tagclients", 4);
    }

    PutTagReaderOnPath();
    sTagReaderThread = new QThread;
    sTagReaderThread->start();
    sTagReader = new TagReaderClient;
    sTagReader->moveToThread(sTagReaderThread);
    sTagReader->Start();
  }

  static void TearDownTestCase() {
    // Deleted on its own thread, which stops the workers.
    QMetaObject::invokeMethod(
        sTagReader, []() { delete sTagReader; }, Qt::BlockingQueuedConnection);
    sTagReaderThread->quit();
    sTagReaderThread->wait();
    delete sTagReaderThread;
    delete sSettingsDir;
  }

  void SetUp() {
    ASSERT_TRUE(library_dir_.isValid());
    // The library keeps canonical paths, and on macOS the temporary
    // directory is under /var, a symlink to /private/var.
    root_ = QFileInfo(library_dir_.path()).canonicalFilePath();

    database_.reset(new MemoryDatabase(nullptr));
    backend_.reset(new HookedLibraryBackend);
    backend_->Init(database_.get(), Library::kSongsTable, Library::kDirsTable,
                   Library::kSubdirsTable, Library::kFtsTable);

    // As Library::Init. Queued like the app's cross-thread connections, so
    // scans run from the event loop rather than inside the backend.
    watcher_.reset(new LibraryWatcher);
    watcher_->set_backend(backend_.get());
    watcher_->set_task_manager(&task_manager_);

    const Qt::ConnectionType queued = Qt::QueuedConnection;
    QObject::connect(backend_.get(),
                     SIGNAL(DirectoryDiscovered(Directory, SubdirectoryList)),
                     watcher_.get(),
                     SLOT(AddDirectory(Directory, SubdirectoryList)), queued);
    QObject::connect(watcher_.get(), SIGNAL(NewOrUpdatedSongs(SongList)),
                     backend_.get(), SLOT(AddOrUpdateSongs(SongList)), queued);
    QObject::connect(watcher_.get(), SIGNAL(SongsMTimeUpdated(SongList)),
                     backend_.get(), SLOT(UpdateMTimesOnly(SongList)), queued);
    QObject::connect(watcher_.get(), SIGNAL(SongsDeleted(SongList)),
                     backend_.get(), SLOT(MarkSongsUnavailable(SongList)),
                     queued);
    QObject::connect(watcher_.get(), SIGNAL(SongsReadded(SongList, bool)),
                     backend_.get(), SLOT(MarkSongsUnavailable(SongList, bool)),
                     queued);
    QObject::connect(
        watcher_.get(), SIGNAL(SubdirsDiscovered(SubdirectoryList)),
        backend_.get(), SLOT(AddOrUpdateSubdirs(SubdirectoryList)), queued);
    QObject::connect(
        watcher_.get(), SIGNAL(SubdirsMTimeUpdated(SubdirectoryList)),
        backend_.get(), SLOT(AddOrUpdateSubdirs(SubdirectoryList)), queued);

    MakeLibrary();
  }

  // Formats/  one beep each as MP3, FLAC and Ogg Vorbis
  // Many/     kManyFiles copies of the MP3
  // Cue/      one FLAC split into two tracks by a cue sheet
  void MakeLibrary() {
    const QString data = TEST_DATA_DIR;
    QDir root(root_);
    ASSERT_TRUE(root.mkpath("Formats") && root.mkpath("Many") &&
                root.mkpath("Cue"));
    for (const char* ext : {"mp3", "flac", "ogg"}) {
      ASSERT_TRUE(
          QFile::copy(data + "/beep." + ext, root_ + "/Formats/beep." + ext));
    }
    for (int i = 0; i < kManyFiles; ++i) {
      ASSERT_TRUE(QFile::copy(data + "/beep.mp3", ManyFile(i)));
    }
    ASSERT_TRUE(QFile::copy(data + "/beep.flac", root_ + "/Cue/whole.flac"));
    QFile cue(root_ + "/Cue/whole.cue");
    ASSERT_TRUE(cue.open(QIODevice::WriteOnly));
    cue.write(
        "PERFORMER \"Cue Artist\"\n"
        "TITLE \"Cue Album\"\n"
        "FILE \"whole.flac\" WAVE\n"
        "  TRACK 01 AUDIO\n"
        "    TITLE \"One\"\n"
        "    INDEX 01 00:00:00\n"
        "  TRACK 02 AUDIO\n"
        "    TITLE \"Two\"\n"
        "    INDEX 01 00:00:10\n");
  }

  QString ManyFile(int i) const {
    return QString("%1/Many/%2.mp3").arg(root_).arg(i, 2, 10, QChar('0'));
  }

  // The available songs, by path and, for cue tracks, where they start.
  QMap<QString, Song> Songs() {
    QMap<QString, Song> songs;
    for (const Song& song : backend_->GetAllSongs()) {
      if (song.is_unavailable()) continue;
      songs[QString("%1#%2")
                .arg(song.url().toLocalFile())
                .arg(song.beginning_nanosec())] = song;
    }
    return songs;
  }

  Song SongAt(const QString& path) { return Songs().value(path + "#0"); }

  bool WaitFor(std::function<bool()> condition) {
    return QTest::qWaitFor(condition, kTimeoutMsec);
  }

  void ScanLibrary() {
    backend_->AddDirectory(root_);
    ASSERT_TRUE(WaitFor([this]() {
      return Songs().size() == 3 + kManyFiles + 2;
    })) << "found "
        << Songs().size() << " songs";
  }

  // Rewrites |path|'s title, and moves its mtime on so a rescan notices.
  void Retag(const QString& path, const QString& title) {
    Song song = SongAt(path);
    song.set_title(title);
    ASSERT_TRUE(TagReaderClient::Instance()->SaveFileBlocking(path, song));

    QFile file(path);
    ASSERT_TRUE(file.open(QIODevice::ReadWrite));
    ASSERT_TRUE(file.setFileTime(QDateTime::currentDateTime().addSecs(10),
                                 QFileDevice::FileModificationTime));
  }

  // Moves a directory's mtime on. A rescan only looks inside directories
  // whose mtime, in whole seconds, changed, and a test is quicker than that.
  // With std::filesystem: on Windows, utime() can't set a directory's.
  void TouchDirectory(const QString& path) {
    const std::filesystem::path dir(path.toStdU16String());
    std::error_code error;
    const std::filesystem::file_time_type mtime =
        std::filesystem::last_write_time(dir, error);
    ASSERT_FALSE(error) << error.message();
    std::filesystem::last_write_time(dir, mtime + std::chrono::seconds(10),
                                     error);
    ASSERT_FALSE(error) << error.message();
  }

  QTemporaryDir library_dir_;
  QString root_;
  std::unique_ptr<Database> database_;
  std::unique_ptr<HookedLibraryBackend> backend_;
  TaskManager task_manager_;
  std::unique_ptr<LibraryWatcher> watcher_;
};

TEST_F(LibraryWatcherTest, FirstScanReadsEveryFile) {
  ScanLibrary();
  QMap<QString, Song> songs = Songs();

  EXPECT_EQ(Song::Type_Mpeg, songs[root_ + "/Formats/beep.mp3#0"].filetype());
  EXPECT_EQ(Song::Type_Flac, songs[root_ + "/Formats/beep.flac#0"].filetype());
  EXPECT_EQ(Song::Type_OggVorbis,
            songs[root_ + "/Formats/beep.ogg#0"].filetype());

  for (int i = 0; i < kManyFiles; ++i) {
    const Song song = songs.value(ManyFile(i) + "#0");
    EXPECT_TRUE(song.is_valid()) << ManyFile(i).toStdString();
    EXPECT_GT(song.length_nanosec(), 0) << ManyFile(i).toStdString();
  }

  // The cue sheet's tracks, not the file itself.
  QStringList cue_titles;
  for (const Song& song : songs) {
    if (song.url().toLocalFile().endsWith("whole.flac")) {
      EXPECT_TRUE(song.has_cue());
      cue_titles << song.title();
    }
  }
  cue_titles.sort();
  EXPECT_EQ(QStringList({"One", "Two"}), cue_titles);
}

TEST_F(LibraryWatcherTest, RescanPicksUpChangedAddedAndDeletedFiles) {
  ScanLibrary();

  Retag(ManyFile(10), "Retagged");
  ASSERT_TRUE(QFile::remove(ManyFile(5)));
  ASSERT_TRUE(QFile::copy(QString(TEST_DATA_DIR) + "/beep.mp3", ManyFile(99)));
  TouchDirectory(root_ + "/Many");

  watcher_->IncrementalScanAsync();
  const bool rescanned = WaitFor([this]() {
    return SongAt(ManyFile(10)).title() == "Retagged" &&
           SongAt(ManyFile(99)).is_valid() && !SongAt(ManyFile(5)).is_valid();
  });
  ASSERT_TRUE(rescanned) << "retagged: "
                         << SongAt(ManyFile(10)).title().toStdString()
                         << ", added: " << SongAt(ManyFile(99)).is_valid()
                         << ", deleted still there: "
                         << SongAt(ManyFile(5)).is_valid();

  // Nothing else changed.
  QMap<QString, Song> songs = Songs();
  EXPECT_EQ(3 + kManyFiles + 2, songs.size());
  EXPECT_TRUE(songs.contains(root_ + "/Formats/beep.ogg#0"));
  EXPECT_NE("Retagged", SongAt(ManyFile(11)).title());
}

// Files can go between the scan listing a directory and reading them, such
// as when they're moved while it runs.
TEST_F(LibraryWatcherTest, RescanCopesWithFilesDeletedAsItRuns) {
  ScanLibrary();

  // Only Many/ has changed, so it's the first directory the rescan lists.
  TouchDirectory(root_ + "/Many");
  backend_->before_find_songs_ = [this]() {
    for (int i = 0; i < kManyFiles; ++i) QFile::remove(ManyFile(i));
  };

  watcher_->IncrementalScanAsync();
  ASSERT_TRUE(WaitFor([this]() { return Songs().size() == 3 + 2; }))
      << Songs().size() << " songs left";
  EXPECT_FALSE(backend_->before_find_songs_);
  for (int i = 0; i < kManyFiles; ++i) {
    EXPECT_FALSE(SongAt(ManyFile(i)).is_valid()) << ManyFile(i).toStdString();
  }
}

TEST_F(LibraryWatcherTest, FullRescanRereadsUnchangedDirectories) {
  ScanLibrary();

  // Rewriting a file in place doesn't change its directory's mtime, so only a
  // full rescan looks at it.
  Retag(root_ + "/Formats/beep.flac", "Retagged");

  watcher_->FullScanAsync();
  ASSERT_TRUE(WaitFor([this]() {
    return SongAt(root_ + "/Formats/beep.flac").title() == "Retagged";
  }));
  EXPECT_EQ(3 + kManyFiles + 2, Songs().size());
}

}  // namespace
