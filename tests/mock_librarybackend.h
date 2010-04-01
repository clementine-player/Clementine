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

#ifndef MOCK_LIBRARYBACKEND_H
#define MOCK_LIBRARYBACKEND_H

#include "librarybackend.h"
#include "test_utils.h"

#include <gmock/gmock.h>

class MockLibraryBackend : public LibraryBackendInterface {
 public:
  ~MockLibraryBackend() { Die(); }
  MOCK_METHOD0(Die, void());

  MOCK_METHOD0(LoadDirectoriesAsync,
      void());
  MOCK_METHOD0(UpdateTotalSongCountAsync,
      void());
  MOCK_METHOD1(FindSongsInDirectory,
      SongList(int id));
  MOCK_METHOD1(GetAllArtists,
      QStringList(const QueryOptions& opt));
  MOCK_METHOD3(GetSongs,
      SongList(const QString& artist, const QString& album, const QueryOptions& opt));
  MOCK_METHOD1(HasCompilations,
      bool(const QueryOptions& opt));
  MOCK_METHOD2(GetCompilationSongs,
      SongList(const QString& album, const QueryOptions& opt));
  MOCK_METHOD1(GetAllAlbums,
      AlbumList(const QueryOptions& opt));
  MOCK_METHOD2(GetAlbumsByArtist,
      AlbumList(const QString& artist, const QueryOptions& opt));
  MOCK_METHOD1(GetCompilationAlbums,
      AlbumList(const QueryOptions& opt));
  MOCK_METHOD3(UpdateManualAlbumArtAsync,
      void(const QString& artist, const QString& album, const QString& art));
  MOCK_METHOD2(GetAlbumArt,
      Album(const QString& artist, const QString& album));
  MOCK_METHOD1(GetSongById,
      Song(int id));
  MOCK_METHOD1(AddDirectory,
      void(const QString& path));
  MOCK_METHOD1(RemoveDirectory,
      void(const Directory& dir));
  MOCK_METHOD0(UpdateCompilationsAsync,
      void());
  MOCK_METHOD0(LoadDirectories,
      void());
  MOCK_METHOD0(UpdateTotalSongCount,
      void());
  MOCK_METHOD1(AddOrUpdateSongs,
      void(const SongList& songs));
  MOCK_METHOD1(UpdateMTimesOnly,
      void(const SongList& songs));
  MOCK_METHOD1(DeleteSongs,
      void(const SongList& songs));
  MOCK_METHOD0(UpdateCompilations,
      void());
  MOCK_METHOD3(UpdateManualAlbumArt,
      void(const QString& artist, const QString& album, const QString& art));
  MOCK_METHOD3(ForceCompilation,
      void(const QString& artist, const QString& album, bool on));

  void ExpectSetup(bool has_compilations = false,
                   const QStringList& artists = QStringList());

  EXPOSE_SIGNAL2(DirectoryDiscovered, Directory, SubdirectoryList);
  EXPOSE_SIGNAL1(DirectoryDeleted, Directory);

  EXPOSE_SIGNAL1(SongsDiscovered, SongList);
  EXPOSE_SIGNAL1(SongsDeleted, SongList);

  EXPOSE_SIGNAL1(TotalSongCountUpdated, int);
};

#endif
