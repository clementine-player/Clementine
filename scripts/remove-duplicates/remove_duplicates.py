import clementine
from clementine import SongInsertVetoListener


class RemoveDuplicatesListener(SongInsertVetoListener):

  def __init__(self):
    SongInsertVetoListener.__init__(self)

  def init_listener(self):
    for playlist in clementine.playlists.GetAllPlaylists():
      playlist.AddSongInsertVetoListener(self)
      
    clementine.playlists.PlaylistAdded.connect(self.playlist_added)

  def remove_duplicates(self):
    for playlist in clementine.playlists.GetAllPlaylists():
      self.remove_duplicates_from(playlist)

  def playlist_added(self, playlist_id):
    playlist = clementine.playlists.playlist(playlist_id)

    playlist.AddSongInsertVetoListener(self)
    self.remove_duplicates_from(playlist)

  def AboutToInsertSongs(self, old_songs, new_songs):
    vetoed = []
    used_urls = set()

    songs = old_songs + new_songs
    for song in songs:
      url = self.url_for_song(song)

      # don't veto songs without URL (possibly radios)
      if len(url) > 0:
        if url in used_urls:
          vetoed.append(song)
        used_urls.add(url)

    return vetoed

  def remove_duplicates_from(self, playlist):
    indices = []
    used_urls = set()

    songs = playlist.GetAllSongs()
    for i in range(0, len(songs)):
      song = songs[i]
      url = self.url_for_song(song)

      # ignore songs without URL (possibly radios)
      if len(url) > 0:
        if url in used_urls:
          indices.append(i)
        used_urls.add(url)

    if len(indices) > 0:
      playlist.RemoveItemsWithoutUndo(indices)

  def url_for_song(self, song):
    if not song.filename() == "":
      return song.filename() + ":" + str(song.beginning_nanosec())
    else:
      return ""
		

script = RemoveDuplicatesListener()

script.init_listener()
script.remove_duplicates()