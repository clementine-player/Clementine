from service        import DigitallyImportedService
from settingsdialog import SettingsDialog

import clementine

class Plugin:
  def __init__(self):
    self.settings_dialog = None

    # Create the service and add it to the Internet tab
    self.service = DigitallyImportedService(clementine.radio_model)
    clementine.radio_model.AddService(self.service)

    # Register for when the user clicks the Settings button
    script.SettingsDialogRequested.connect(self.ShowSettings)

    # Register for the "Configure..." right click menu item
    self.service.SettingsDialogRequested.connect(self.ShowSettings)

  def ShowSettings(self):
    if not self.settings_dialog:
      # Create the dialog the first time it's shown
      self.settings_dialog = SettingsDialog()
      self.settings_dialog.accepted.connect(self.service.ReloadSettings)

    self.settings_dialog.show()

plugin = Plugin()
