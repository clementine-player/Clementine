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

#ifndef LIBRARY_H
#define LIBRARY_H

#include "core/backgroundthread.h"

#include <QObject>

#include <boost/scoped_ptr.hpp>

class Database;
class LibraryBackend;
class LibraryModel;
class LibraryWatcher;

class Library : public QObject {
  Q_OBJECT

 public:
  Library(Database* db, QObject* parent);

  static const char* kSongsTable;
  static const char* kDirsTable;
  static const char* kSubdirsTable;

  // Useful for tests.  The library takes ownership.
  void set_watcher_factory(BackgroundThreadFactory<LibraryWatcher>* factory);

  void Init();
  void StartThreads();

  LibraryModel* model() const { return model_; }

 private slots:
  void WatcherInitialised();

 private:
  LibraryBackend* backend_;
  LibraryModel* model_;

  boost::scoped_ptr<BackgroundThreadFactory<LibraryWatcher> > watcher_factory_;
  BackgroundThread<LibraryWatcher>* watcher_;

 signals:
  void ScanStarted();
  void ScanFinished();
};

#endif
