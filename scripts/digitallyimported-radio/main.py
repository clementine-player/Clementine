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
    # Create the dialog the first time it's shown
    if not self.settings_dialog:
      self.settings_dialog = SettingsDialog()

    # Show the dialog
    self.settings_dialog.show()

plugin = Plugin()
