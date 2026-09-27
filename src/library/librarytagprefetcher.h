/* This file is part of Clementine.
   Copyright 2026, John Maguire <john.maguire@gmail.com>

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

#ifndef LIBRARY_LIBRARYTAGPREFETCHER_H_
#define LIBRARY_LIBRARYTAGPREFETCHER_H_

#include <QHash>
#include <QString>
#include <QStringList>

#include "core/tagreaderclient.h"

class Song;

// Asks the tag reader for a directory's files ahead of the loop that compares
// them with the library, keeping a window of requests in flight so the tag
// reader's workers read several files at once. The loop still reads files in
// order and one at a time; it just rarely has to wait.
class LibraryTagPrefetcher {
 public:
  // What the prefetcher needs from the tag reader.
  class Reader {
   public:
    virtual ~Reader() {}
    // Starts reading |file|'s tags; returns an id for Finish or Cancel.
    virtual int Start(const QString& file) = 0;
    // Waits for the read Start returned |id| for and fills |song|.
    virtual void Finish(int id, const QString& file, Song* song) = 0;
    // The result of |id| won't be wanted.
    virtual void Cancel(int id) = 0;
    // Reads |file|'s tags now, without a prefetch.
    virtual void ReadNow(const QString& file, Song* song) = 0;
  };

  // Requests beyond this wait until earlier ones are used, so a directory of
  // thousands of files doesn't queue thousands of requests.
  static const int kMaxInFlight;

  // |files| are those the loop is expected to read, in the order it will.
  // |reader| must outlive the prefetcher.
  LibraryTagPrefetcher(Reader* reader, const QStringList& files);
  // Cancels requests the loop didn't use.
  ~LibraryTagPrefetcher();

  // Reads |file|'s tags: from its prefetched request if there is one,
  // otherwise directly.
  void Read(const QString& file, Song* song);

  int in_flight() const { return in_flight_.size(); }

 private:
  struct InFlight {
    int index;
    int id;
  };

  void TopUp();

  Reader* reader_;
  QStringList files_;
  QHash<QString, int> index_;
  int next_;
  QHash<QString, InFlight> in_flight_;
};

// The Reader Clementine uses: TagReaderClient's asynchronous requests.
class TagReaderClientPrefetchReader : public LibraryTagPrefetcher::Reader {
 public:
  int Start(const QString& file);
  void Finish(int id, const QString& file, Song* song);
  void Cancel(int id);
  void ReadNow(const QString& file, Song* song);

 private:
  QHash<int, TagReaderClient::ReplyType*> replies_;
  int next_id_ = 1;
};

#endif  // LIBRARY_LIBRARYTAGPREFETCHER_H_
