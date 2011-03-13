from PyQt4.Qt import QAction
from PyQt4.QtCore import QObject
from PyQt4.QtCore import Qt
from PyQt4.QtCore import SIGNAL

from PyQt4.QtGui import QColor

import clementine


class RainbowizerScript(QObject):

  priority = 1
  colors = [ QColor("#ec1e24"),
             QColor("#f45a2c"),
             QColor("#fcf204"),
             QColor("#3cb64c"),
             QColor("#04aeec"),
             QColor("#242264"),
             QColor("#94268c") ];

  def __init__(self):
    QObject.__init__(self)

    self.action = QAction("rainbowize_playlist", self)
    self.action.setText("Rainbowize!")
    self.action.setCheckable(True)
    self.connect(self.action, SIGNAL("changed()"), self.rainbowize)

    clementine.ui.AddAction('playlist_menu', self.action)

  def rainbowize(self):
    for playlist in clementine.playlists.GetAllPlaylists():
      if self.action.isChecked():
        i = 0

        for item in playlist.GetAllItems():
          i = (i + 1) % len(self.colors)
          item.SetBackgroundColor(self.priority, self.colors[i])

      else:
        # undo all rainbow colors
        for item in playlist.GetAllItems():
          item.RemoveBackgroundColor(self.priority)


script = RainbowizerScript()
