from servicebase import DigitallyImportedServiceBase

from PyQt4.QtCore import QSettings
from PyQt4.QtGui  import QDialog, QIcon
import PyQt4.uic

import os.path

class SettingsDialog(QDialog):
  def __init__(self, parent=None):
    QDialog.__init__(self, parent)

    self.path = os.path.dirname(__file__)

    # Set up the user interface
    PyQt4.uic.loadUi(os.path.join(self.path, "settingsdialog.ui"), self)

    # Set the window icon
    self.setWindowIcon(QIcon(os.path.join(self.path, "icon-small.png")))

  def showEvent(self, event):
    # Load the settings
    settings = QSettings()
    settings.beginGroup(DigitallyImportedServiceBase.SETTINGS_GROUP)
    self.type.setCurrentIndex(int(settings.value("audio_type", 0).toPyObject()))
    self.username.setText(settings.value("username", "").toPyObject())
    self.password.setText(settings.value("password", "").toPyObject())

    QDialog.showEvent(self, event)

  def accept(self):
    # Save the settings
    settings = QSettings()
    settings.beginGroup(DigitallyImportedServiceBase.SETTINGS_GROUP)
    settings.setValue("audio_type", self.type.currentIndex())
    settings.setValue("username", self.username.text())
    settings.setValue("password", self.password.text())

    QDialog.accept(self)
