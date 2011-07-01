import clementine


class RemoveDuplicatesListener(clementine.SongInsertVetoListener):

  def __init__(self):
    clementine.SongInsertVetoListener.__init__(self)

  def init_listener(self):
    for playlist in clementine.playlists.GetAllPlaylists():
      playlist.AddSongInsertVetoListener(self)

    clementine.playlists.connect("PlaylistAdded(int,QString)", self.playlist_added)   

  def remove_duplicates(self):
    for playlist in clementine.playlists.GetAllPlaylists():
      self.remove_duplicates_from(playlist)

  def playlist_added(self, playlist_id, playlist_name):
    playlist = clementine.playlists.playlist(playlist_id)

    playlist.AddSongInsertVetoListener(self)
    self.remove_duplicates_from(playlist)

  def AboutToInsertSongs(self, old_songs, new_songs):
    return [song for song in new_songs if song in old_songs]

  def remove_duplicates_from(self, playlist):
    duplicate_indices = []
    uniques = []
    songs = playlist.GetAllSongs()

    for index in range(0, len(songs)):
      song = songs[index]
      if song not in uniques:
        uniques.append(song)
      else:
        duplicate_indices.append(index)

    if len(duplicate_indices) > 0:
      playlist.RemoveItemsWithoutUndo(duplicate_indices)


script = RemoveDuplicatesListener()
script.init_listener()
script.remove_duplicates()
