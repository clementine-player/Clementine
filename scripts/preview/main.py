import clementine

from PyQt4.QtCore import QTimer
from PyQt4.QtGui import QAction

class Plugin:
  def __init__(self):
    self.timer = QTimer(None)
    self.timer.setInterval(10000)
    self.timer.timeout.connect(self.Timeout)
    self.timer.setSingleShot(True)
    self.action = QAction("Preview mode", None)
    self.action.setCheckable(True)
    self.action.triggered.connect(self.Enabled)
    clementine.ui.AddAction("playlist_menu", self.action)
    clementine.player.Playing.connect(self.Playing)
    clementine.player.Paused.connect(self.Stopped)
    clementine.player.Stopped.connect(self.Stopped)
    self.enabled = False

  def Enabled(self, enabled):
    self.enabled = enabled
    if enabled:
      if clementine.player.GetState() == 2:  # Playing
        self.timer.start()
    else:
      self.timer.stop()

  def Playing(self):
    if self.enabled:
      self.timer.start()

  def Stopped(self):
    self.timer.stop()

  def Timeout(self):
    if clementine.player.GetState() == 2:
      clementine.player.Next()


plugin = Plugin()
