import clementine

from PyQt4.QtGui import QAction

import sys

class Plugin:
  def __init__(self):
    self.enabled = False
    clementine.player.PlaylistFinished.connect(self.PlaylistFinished)
    self.action = QAction("Shutdown at end", None)
    self.action.setCheckable(True)
    self.action.triggered.connect(self.Enabled)
    clementine.ui.AddAction("playlist_menu", self.action)


  # Slots
  def PlaylistFinished(self):
    if self.enabled:
      print "Reached the end of the playlist - shutting down."
      sys.exit(0)

  def Enabled(self, enabled):
    print "Shutdown at end of playlist enabled: %s" % enabled
    self.enabled = enabled

plugin = Plugin()
