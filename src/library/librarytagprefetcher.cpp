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

#include "librarytagprefetcher.h"

const int LibraryTagPrefetcher::kMaxInFlight = 32;

LibraryTagPrefetcher::LibraryTagPrefetcher(Reader* reader,
                                           const QStringList& files)
    : reader_(reader), files_(files), next_(0) {
  for (int i = 0; i < files_.size(); ++i) index_[files_[i]] = i;
  TopUp();
}

LibraryTagPrefetcher::~LibraryTagPrefetcher() {
  for (const InFlight& request : in_flight_) reader_->Cancel(request.id);
}

void LibraryTagPrefetcher::Read(const QString& file, Song* song) {
  const int index = index_.value(file, -1);

  // Requests for files the loop has passed without reading will never be
  // used, and would hold the window up.
  if (index != -1) {
    for (auto it = in_flight_.begin(); it != in_flight_.end();) {
      if (it->index < index) {
        reader_->Cancel(it->id);
        it = in_flight_.erase(it);
      } else {
        ++it;
      }
    }
  }

  auto it = in_flight_.find(file);
  if (it == in_flight_.end()) {
    // Not prefetched: read it now, and prefetch from here on.
    if (index >= next_) next_ = index + 1;
    TopUp();
    reader_->ReadNow(file, song);
    return;
  }

  // Finish before refilling the window, so at most kMaxInFlight requests are
  // ever open.
  const int id = it->id;
  in_flight_.erase(it);
  reader_->Finish(id, file, song);
  TopUp();
}

void LibraryTagPrefetcher::TopUp() {
  while (next_ < files_.size() && in_flight_.size() < kMaxInFlight) {
    const QString& file = files_[next_];
    in_flight_[file] = InFlight{next_, reader_->Start(file)};
    ++next_;
  }
}

int TagReaderClientPrefetchReader::Start(const QString& file) {
  const int id = next_id_++;
  replies_[id] = TagReaderClient::Instance()->ReadFile(file);
  return id;
}

void TagReaderClientPrefetchReader::Finish(int id, const QString& file,
                                           Song* song) {
  TagReaderClient::Instance()->ReadFileFinish(replies_.take(id), file, song);
}

void TagReaderClientPrefetchReader::Cancel(int id) {
  replies_.take(id)->deleteLater();
}

void TagReaderClientPrefetchReader::ReadNow(const QString& file, Song* song) {
  TagReaderClient::Instance()->ReadFileBlocking(file, song);
}
