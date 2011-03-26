import clementine

from PyQt4.QtCore import QObject
from PyQt4.QtCore import SIGNAL
from PyQt4.QtGui import QAction

class InvalidateDeleted(QObject):

  def __init__(self):
    QObject.__init__(self)

    self.action = QAction("invalidate_deleted", self)
    self.action.setText("Grey out deleted songs")
    self.connect(self.action, SIGNAL("activated()"), self.grey_out_activated)

    clementine.ui.AddAction('playlist_menu', self.action)

  def grey_out_activated(self):
    clementine.playlists.InvalidateDeletedSongs()
		

script = InvalidateDeleted()
