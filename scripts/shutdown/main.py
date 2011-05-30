import clementine

from PythonQt.QtGui import QAction

import logging
import sys

LOGGER = logging.getLogger("system-shutdown")


class Plugin:
  def __init__(self):
    self.enabled = False
    clementine.player.connect("PlaylistFinished()", self.PlaylistFinished)
    self.action = QAction("Shutdown at end", None)
    self.action.setCheckable(True)
    self.action.connect("triggered(bool)", self.Enabled)
    clementine.ui.AddAction("playlist_menu", self.action)

  def PlaylistFinished(self):
    if self.enabled:
      LOGGER.info("Reached the end of the playlist - shutting down")
      sys.exit(0)

  def Enabled(self, enabled):
    LOGGER.info("Shutdown at end of playlist enabled: %s" % str(enabled))
    self.enabled = enabled


plugin = Plugin()
