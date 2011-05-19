from servicebase import DigitallyImportedServiceBase

from PythonQt.QtCore    import QEvent, QFile, QObject, QSettings
from PythonQt.QtGui     import QComboBox, QDialog, QIcon, QLineEdit
from PythonQt.QtUiTools import QUiLoader

import os.path

class SettingsDialog(QObject):
  def __init__(self, parent=None):
    QObject.__init__(self, parent)

    self.path = os.path.dirname(__file__)

    # Set up the user interface
    ui_filename = os.path.join(self.path, "settingsdialog.ui")
    ui_file = QFile(ui_filename)
    if not ui_file.open(QFile.ReadOnly):
      raise IOError(ui_file)

    loader = QUiLoader()
    self.dialog = loader.load(ui_file)

    # Get the widgets from the dialog so we can use them later
    self.type = self.dialog.findChild(QComboBox, "type")
    self.username = self.dialog.findChild(QLineEdit, "username")
    self.password = self.dialog.findChild(QLineEdit, "password")

    # Install ourselves as an event filter on the dialog so we can see when
    # it gets shown
    self.dialog.installEventFilter(self)

    # Connect to the accepted signal so we can save settings when the user
    # clicks OK
    self.dialog.connect("accepted()", self.SaveSettings)

    # Set the window icon
    self.dialog.setWindowIcon(QIcon(os.path.join(self.path, "icon-small.png")))

  def eventFilter(self, obj, event):
    if obj == self.dialog:
      if event.type() == QEvent.Show:
        self.LoadSettings()

    QObject.eventFilter(self, obj, event)

  def LoadSettings(self):
    settings = QSettings()
    settings.beginGroup(DigitallyImportedServiceBase.SETTINGS_GROUP)
    self.type.setCurrentIndex(int(settings.value("audio_type", 0)))
    self.username.setText(settings.value("username", ""))
    self.password.setText(settings.value("password", ""))

  def SaveSettings(self):
    settings = QSettings()
    settings.beginGroup(DigitallyImportedServiceBase.SETTINGS_GROUP)
    settings.setValue("audio_type", self.type.currentIndex)
    settings.setValue("username", self.username.text)
    settings.setValue("password", self.password.text)
