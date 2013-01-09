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

#ifndef MOCKLIBRARYBACKEND_H
#define MOCKLIBRARYBACKEND_H

#include <gmock/gmock.h>

#include "library/librarybackend.h"

class MockLibraryBackend : public LibraryBackendInterface {
 public:
  MOCK_CONST_METHOD0(songs_table, QString());

  // Get a list of directories in the library.  Emits DirectoriesDiscovered.
  MOCK_METHOD0(LoadDirectoriesAsync, void());

  // Counts the songs in the library.  Emits TotalSongCountUpdated
  MOCK_METHOD0(UpdateTotalSongCountAsync, void());

  MOCK_METHOD1(FindSongsInDirectory, SongList(int));
  MOCK_METHOD1(SubdirsInDirectory, SubdirectoryList(int));
  MOCK_METHOD0(GetAllDirectories, DirectoryList());
  MOCK_METHOD3(ChangeDirPath, void(int, const QString&, const QString&));

  MOCK_METHOD1(GetAllArtists, QStringList(const QueryOptions&));
  MOCK_METHOD1(GetAllArtistsWithAlbums, QStringList(const QueryOptions&));
  MOCK_METHOD3(GetSongs, SongList(const QString&, const QString&, const QueryOptions&));

  MOCK_METHOD2(GetCompilationSongs, SongList(const QString&, const QueryOptions&));

  MOCK_METHOD1(GetAllAlbums, AlbumList(const QueryOptions&));
  MOCK_METHOD2(GetAlbumsByArtist, AlbumList(const QString&, const QueryOptions&));
  MOCK_METHOD1(GetCompilationAlbums, AlbumList(const QueryOptions&));

  MOCK_METHOD3(UpdateManualAlbumArtAsync, void(const QString&, const QString&, const QString&));
  MOCK_METHOD2(GetAlbumArt, Album(const QString&, const QString&));

  MOCK_METHOD1(GetSongById, Song(int));

  MOCK_METHOD1(GetSongsByUrl, SongList(const QUrl&));
  MOCK_METHOD2(GetSongByUrl, Song(const QUrl&, qint64));

  MOCK_METHOD1(AddDirectory, void(const QString&));
  MOCK_METHOD1(RemoveDirectory, void(const Directory&));

  MOCK_METHOD1(ExecQuery, bool(LibraryQuery*));
};

#endif
