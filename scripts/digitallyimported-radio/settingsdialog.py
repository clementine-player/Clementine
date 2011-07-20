from servicebase import DigitallyImportedServiceBase

from PythonQt.QtCore    import QEvent, QFile, QSettings
from PythonQt.QtGui     import QComboBox, QDialog, QIcon, QLineEdit
import uic

import os.path

class SettingsDialog(QDialog):
  def __init__(self, parent=None):
    QDialog.__init__(self, parent)

    self.path = os.path.dirname(__file__)

    # Set up the user interface
    uic.loadUi(os.path.join(self.path, "settingsdialog.ui"), self)

    # Set the window icon
    self.setWindowIcon(QIcon(os.path.join(self.path, "icon-small.png")))

  def showEvent(self, event):
    # Load the settings
    settings = QSettings()
    settings.beginGroup(DigitallyImportedServiceBase.SETTINGS_GROUP)
    self.type.setCurrentIndex(int(settings.value("audio_type", 0)))
    self.username.setText(settings.value("username", ""))
    self.password.setText(settings.value("password", ""))

    #QDialog.showEvent(self, event)

  def accept(self):
    # Save the settings
    settings = QSettings()
    settings.beginGroup(DigitallyImportedServiceBase.SETTINGS_GROUP)
    settings.setValue("audio_type", self.type.currentIndex)
    settings.setValue("username", self.username.text)
    settings.setValue("password", self.password.text)

    QDialog.done(self, QDialog.Accepted)
