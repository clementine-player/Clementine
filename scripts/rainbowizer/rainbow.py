from PythonQt.QtCore import QObject
from PythonQt.QtGui  import QAction, QColor

import clementine


class RainbowizerScript(QObject):
  PRIORITY = 1

  def __init__(self):
    QObject.__init__(self)

    # Generate colors
    self.colors = []
    for hue in xrange(0, 255, 30):
      self.colors.append(QColor.fromHsv(hue, 255, 255, 96))

    self.action = QAction("rainbowize_playlist", self)
    self.action.setText("Rainbowize!")
    self.action.setCheckable(True)
    self.action.connect("changed()", self.rainbowize)

    clementine.ui.AddAction('playlist_menu', self.action)

  def rainbowize(self):
    for playlist in clementine.playlists.GetAllPlaylists():
      if self.action.isChecked():
        for i, item in enumerate(playlist.GetAllItems()):
          item.SetBackgroundColor(self.PRIORITY, self.colors[i % len(self.colors)])

      else:
        # undo all rainbow colors
        for item in playlist.GetAllItems():
          item.RemoveBackgroundColor(self.PRIORITY)


script = RainbowizerScript()
