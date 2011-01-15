from PyQt4.QtGui import QDialog, QIcon
import PyQt4.uic

import os.path

class SettingsDialog(QDialog):
  def __init__(self, parent=None):
    QDialog.__init__(self, parent)

    self.path = os.path.dirname(__file__)

    # Set up the user interface
    PyQt4.uic.loadUi(os.path.join(self.path, "settingsdialog.ui"), self)

    # Set the icon
    self.setWindowIcon(QIcon(os.path.join(self.path, "icon-small.png")))
