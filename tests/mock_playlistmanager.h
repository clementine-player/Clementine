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

#ifndef MOCK_PLAYLISTMANAGER_H
#define MOCK_PLAYLISTMANAGER_H

#include "playlist/playlistmanager.h"

class MockPlaylistManager : public PlaylistManagerInterface {
public:
  MockPlaylistManager(QObject* parent = NULL) : PlaylistManagerInterface(parent) {}

  MOCK_CONST_METHOD0(current_id, int());
  MOCK_CONST_METHOD0(active_id, int());

  MOCK_CONST_METHOD1(playlist, Playlist*(int));
  MOCK_CONST_METHOD0(current, Playlist*());
  MOCK_CONST_METHOD0(active, Playlist*());

  MOCK_CONST_METHOD0(GetAllPlaylists, QList<Playlist*>());
  MOCK_METHOD0(InvalidateDeletedSongs, void());
  MOCK_METHOD0(RemoveDeletedSongs, void());

  MOCK_CONST_METHOD1(selection, QItemSelection(int));
  MOCK_CONST_METHOD0(current_selection, QItemSelection());
  MOCK_CONST_METHOD0(active_selection, QItemSelection());

  MOCK_CONST_METHOD1(GetPlaylistName, QString(int));
  MOCK_CONST_METHOD0(task_manager, TaskManager*());
  MOCK_CONST_METHOD0(library_backend, LibraryBackend*());
  MOCK_CONST_METHOD0(playlist_backend, PlaylistBackend*());
  MOCK_CONST_METHOD0(sequence, PlaylistSequence*());
  MOCK_CONST_METHOD0(parser, PlaylistParser*());
  MOCK_CONST_METHOD0(playlist_container, PlaylistContainer*());

  MOCK_METHOD1(RegisterSpecialPlaylistType, void(SpecialPlaylistType*));
  MOCK_METHOD1(UnregisterSpecialPlaylistType, void(SpecialPlaylistType*));
  MOCK_CONST_METHOD1(GetPlaylistType, SpecialPlaylistType*(const QString&));

  MOCK_METHOD3(New, void(const QString&, const SongList&, const QString&));
  MOCK_METHOD1(Load, void(const QString&));
  MOCK_METHOD2(Save, void(int, const QString&));
  MOCK_METHOD2(Rename, void(int, const QString&));
  MOCK_METHOD1(Remove, void(int));
  MOCK_METHOD1(ChangePlaylistOrder, void(const QList<int>&));

  MOCK_METHOD1(SetCurrentPlaylist, void(int));
  MOCK_METHOD1(SetActivePlaylist, void(int));
  MOCK_METHOD0(SetActiveToCurrent, void());

  MOCK_METHOD1(SelectionChanged, void(const QItemSelection&));

  MOCK_METHOD0(ClearCurrent, void());
  MOCK_METHOD0(ShuffleCurrent, void());
  MOCK_METHOD0(SetActivePlaying, void());
  MOCK_METHOD0(SetActivePaused, void());
  MOCK_METHOD0(SetActiveStopped, void());
  MOCK_METHOD2(SetActiveStreamMetadata, void(const QUrl&, const Song&));
  MOCK_METHOD1(RateCurrentSong, void(double));
  MOCK_METHOD1(RateCurrentSong, void(int));

  MOCK_METHOD2(SongChangeRequestProcessed, void(const QUrl& url, bool));

  MOCK_METHOD3(PlaySmartPlaylist, void(smart_playlists::GeneratorPtr, bool, bool));

  void EmitPlaylistManagerInitialized() {
    emit PlaylistManagerInitialized();
  }
};

#endif // MOCK_PLAYLISTMANAGER_H
