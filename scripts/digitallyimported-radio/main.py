from diservice      import DigitallyImportedService
from skyservice     import SkyFmService
from settingsdialog import SettingsDialog

import clementine

class Plugin:
  def __init__(self):
    self.settings_dialog = None

    cb = self.ShowSettings

    # Register for when the user clicks the Settings button
    __script__.connect("SettingsDialogRequested()", cb)

    # Create the services and add them to the Internet tab
    self.di_service = DigitallyImportedService(clementine.internet_model, cb)
    self.sky_service = SkyFmService(clementine.internet_model, cb)
    clementine.internet_model.AddService(self.di_service)
    clementine.internet_model.AddService(self.sky_service)

  def ShowSettings(self):
    if not self.settings_dialog:
      # Create the dialog the first time it's shown
      self.settings_dialog = SettingsDialog()
      self.settings_dialog.connect("accepted()", self.di_service.ReloadSettings)
      self.settings_dialog.connect("accepted()", self.sky_service.ReloadSettings)

    self.settings_dialog.show()

plugin = Plugin()
