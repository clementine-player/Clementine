#include "shortcutsdialog.h"

const char* ShortcutsDialog::kSettingsGroup = "Shortcuts";
ShortcutsDialog::ShortcutsDialog(QWidget* parent)
  : QDialog(parent) {
  ui_.setupUi(this);
  ui_.shortcut_options->setEnabled(false);

  // Load settings
  // Check if settings exist first, if not create them
  // TODO: How do we store the button values? Numbers, strings, especially when it comes to combinations?

  connect(ui_.button_defaults, SIGNAL(clicked()), SLOT(DefaultShortcuts()));
  connect(ui_.button_save, SIGNAL(clicked()), SLOT(SaveShortcuts()));
  connect(ui_.button_cancel, SIGNAL(clicked()), SLOT(CancelEvent()));
}

/**
  * Reset shortcuts to defaults (none for now).
  */
void ShortcutsDialog::DefaultShortcuts() {
}

/**
  * Save the shortcuts and close the window
  */
void ShortcutsDialog::SaveShortcuts() {
  close();
}

/**
  * Reset settings to original values taken from settings file and then close the window.
  */
void ShortcutsDialog::CancelEvent() {
  close();
}
