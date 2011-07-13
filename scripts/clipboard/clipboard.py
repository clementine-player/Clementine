import clementine

from PythonQt.QtGui import QAction
from PythonQt.QtGui import QApplication

class Plugin:
  def __init__(self):
    self.clipboard = QApplication.clipboard()
    self.action = QAction("Copy to clipboard", None)
    clementine.ui.AddAction("song_menu", self.action)
    self.action.connect("activated()", self.CopyToClipboard)

  def CopyToClipboard(self):
    selection = clementine.playlists.current_selection().indexes()
    title = selection[clementine.Playlist.Column_Title].data()
    artist = selection[clementine.Playlist.Column_Artist].data()
    song = '%s - %s' % (title, artist)
    self.clipboard.setText(song)


plugin = Plugin()
