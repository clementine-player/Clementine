import clementine

from PythonQt.QtGui import QAction
from PythonQt.Qt    import QImage

import os

class Plugin:
  def __init__(self):
    self.path = os.path.dirname(__file__)

    clementine.player.connect("SongChangeRequestProcessed(QUrl, bool)", self.Nostalgia)
    self.action = QAction("Nostalgia-ize!", None)
    self.action.setCheckable(True)
    self.action.connect("changed()", self.Nostalgia)

    clementine.ui.AddAction("extras_menu", self.action)
    self.title = "Never Gonna' Give You Up"
    self.artist = "Rick Astley"
    self.album = "Whenever You Need Somebody"
    self.image = QImage(os.path.join(self.path, "nostalgia.jpg"))

  def Nostalgia(self):
    if self.action.isChecked():
      for item in clementine.playlists.current().GetAllItems():
        temp = clementine.Song(item.Metadata())
        temp.set_title(self.title)
        temp.set_artist(self.artist)
        temp.set_album(self.album)
        temp.set_image(self.image)
        item.SetTemporaryMetadata(temp)
    else:
      for item in clementine.playlists.current().GetAllItems():
        item.ClearTemporaryMetadata()
    clementine.playlists.current().PlaylistChanged()

plugin = Plugin()
