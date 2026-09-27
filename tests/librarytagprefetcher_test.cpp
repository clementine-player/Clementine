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

#include "library/librarytagprefetcher.h"

#include <QSet>

#include "core/song.h"
#include "gtest/gtest.h"
#include "test_utils.h"

namespace {

// Records what the prefetcher asks for, and "reads" a file by setting its
// title to the file name.
class FakeReader : public LibraryTagPrefetcher::Reader {
 public:
  int Start(const QString& file) {
    started << file;
    const int id = next_id_++;
    files_[id] = file;
    open_ << id;
    max_open = qMax(max_open, open_.size());
    return id;
  }

  void Finish(int id, const QString& file, Song* song) {
    EXPECT_TRUE(open_.remove(id)) << "finished a request that wasn't open";
    EXPECT_EQ(files_[id], file);
    finished << file;
    song->set_title(file);
  }

  void Cancel(int id) {
    EXPECT_TRUE(open_.remove(id)) << "cancelled a request that wasn't open";
    cancelled << files_[id];
  }

  void ReadNow(const QString& file, Song* song) {
    read_now << file;
    song->set_title(file);
  }

  int open() const { return open_.size(); }

  QStringList started;
  QStringList finished;
  QStringList cancelled;
  QStringList read_now;
  int max_open = 0;

 private:
  int next_id_ = 1;
  QHash<int, QString> files_;
  QSet<int> open_;
};

// Cancellation order doesn't matter.
QStringList Sorted(QStringList list) {
  list.sort();
  return list;
}

QStringList Files(int count) {
  QStringList files;
  for (int i = 0; i < count; ++i) files << QString("/music/%1.mp3").arg(i);
  return files;
}

}  // namespace

TEST(LibraryTagPrefetcherTest, ReadsInOrderFromPrefetchedRequests) {
  FakeReader reader;
  const QStringList files = Files(5);
  {
    LibraryTagPrefetcher prefetcher(&reader, files);
    // Everything is requested before the first read.
    EXPECT_EQ(files, reader.started);

    for (const QString& file : files) {
      Song song;
      prefetcher.Read(file, &song);
      EXPECT_EQ(file, song.title());
    }
    EXPECT_EQ(0, prefetcher.in_flight());
  }
  EXPECT_EQ(files, reader.finished);
  EXPECT_TRUE(reader.read_now.isEmpty());
  EXPECT_TRUE(reader.cancelled.isEmpty());
  EXPECT_EQ(0, reader.open());
}

TEST(LibraryTagPrefetcherTest, KeepsAWindowOfRequests) {
  FakeReader reader;
  const QStringList files = Files(100);
  LibraryTagPrefetcher prefetcher(&reader, files);
  EXPECT_EQ(LibraryTagPrefetcher::kMaxInFlight, reader.started.size());

  Song song;
  for (int i = 0; i < 10; ++i) prefetcher.Read(files[i], &song);
  // Each read makes room for one more request, in order.
  EXPECT_EQ(files.mid(0, LibraryTagPrefetcher::kMaxInFlight + 10),
            reader.started);

  for (int i = 10; i < files.size(); ++i) prefetcher.Read(files[i], &song);
  EXPECT_EQ(files, reader.started);
  EXPECT_EQ(files, reader.finished);
  EXPECT_EQ(LibraryTagPrefetcher::kMaxInFlight, reader.max_open);
}

TEST(LibraryTagPrefetcherTest, CancelsFilesTheLoopSkips) {
  FakeReader reader;
  const QStringList files = Files(10);
  LibraryTagPrefetcher prefetcher(&reader, files);

  // The loop decided files 1 to 3 didn't need reading after all.
  Song song;
  prefetcher.Read(files[0], &song);
  prefetcher.Read(files[4], &song);

  EXPECT_EQ(QStringList({files[1], files[2], files[3]}),
            Sorted(reader.cancelled));
  EXPECT_EQ(QStringList({files[0], files[4]}), reader.finished);
  EXPECT_EQ(5, prefetcher.in_flight());
}

TEST(LibraryTagPrefetcherTest, SkippedFilesDontHoldUpTheWindow) {
  FakeReader reader;
  const QStringList files = Files(LibraryTagPrefetcher::kMaxInFlight * 3);
  LibraryTagPrefetcher prefetcher(&reader, files);

  // Read only every other file. Without cancelling the skipped ones the
  // window would fill with requests nobody will finish.
  Song song;
  for (int i = 0; i < files.size(); i += 2) prefetcher.Read(files[i], &song);

  EXPECT_TRUE(reader.read_now.isEmpty()) << "a file had to be read directly";
  EXPECT_EQ(files.size() / 2, reader.finished.size());
}

TEST(LibraryTagPrefetcherTest, ReadsUnexpectedFilesDirectly) {
  FakeReader reader;
  const QStringList files = Files(3);
  LibraryTagPrefetcher prefetcher(&reader, files);

  // A file that wasn't predicted, such as one whose album art changed.
  Song song;
  prefetcher.Read("/music/unexpected.mp3", &song);
  EXPECT_EQ("/music/unexpected.mp3", song.title());
  EXPECT_EQ(QStringList({"/music/unexpected.mp3"}), reader.read_now);
  // The prefetched files are untouched.
  EXPECT_TRUE(reader.cancelled.isEmpty());
  EXPECT_EQ(3, prefetcher.in_flight());
}

TEST(LibraryTagPrefetcherTest, JumpingAheadOfTheWindowReadsDirectly) {
  FakeReader reader;
  const QStringList files = Files(LibraryTagPrefetcher::kMaxInFlight + 20);
  LibraryTagPrefetcher prefetcher(&reader, files);

  // Everything so far was skipped, and this file wasn't requested yet.
  const int far = LibraryTagPrefetcher::kMaxInFlight + 5;
  Song song;
  prefetcher.Read(files[far], &song);

  EXPECT_EQ(QStringList({files[far]}), reader.read_now);
  EXPECT_EQ(LibraryTagPrefetcher::kMaxInFlight, reader.cancelled.size());
  // Prefetching carries on after it, not from where it was.
  EXPECT_EQ(files[far + 1], reader.started[LibraryTagPrefetcher::kMaxInFlight]);
  EXPECT_FALSE(reader.started.mid(LibraryTagPrefetcher::kMaxInFlight)
                   .contains(files[far]));
}

TEST(LibraryTagPrefetcherTest, CancelsWhatsLeftWhenDestroyed) {
  FakeReader reader;
  const QStringList files = Files(5);
  {
    LibraryTagPrefetcher prefetcher(&reader, files);
    Song song;
    prefetcher.Read(files[0], &song);
  }
  EXPECT_EQ(files.mid(1), Sorted(reader.cancelled));
  EXPECT_EQ(0, reader.open());
}

TEST(LibraryTagPrefetcherTest, NothingToRead) {
  FakeReader reader;
  {
    LibraryTagPrefetcher prefetcher(&reader, QStringList());
    Song song;
    prefetcher.Read("/music/0.mp3", &song);
  }
  EXPECT_TRUE(reader.started.isEmpty());
  EXPECT_EQ(QStringList({"/music/0.mp3"}), reader.read_now);
}
