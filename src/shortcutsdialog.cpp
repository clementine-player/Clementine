#include "shortcutsdialog.h"

const char* ShortcutsDialog::kSettingsGroup = "Shortcuts";
ShortcutsDialog::ShortcutsDialog(QWidget* parent)
  : QDialog(parent) {
  ui_.setupUi(this);
  ui_.shortcut_options->setEnabled(false);

  // Load settings
  // Check if settings exist first, if not create them
  // TODO: QKeySequence::toString() to load/save values

  connect(ui_.button_defaults, SIGNAL(clicked()), SLOT(DefaultShortcuts()));
  connect(ui_.button_box, SIGNAL(accepted()), SLOT(SaveShortcuts()));
  connect(ui_.button_box, SIGNAL(rejected()), SLOT(CancelEvent()));
}

/**
  * Reset shortcuts to defaults (none for now).
  */
void ShortcutsDialog::DefaultShortcuts() {
}

void ShortcutsDialog::SaveShortcuts() {
  accept();
}

/**
  * Reset back to original values found in settings file and close
  */
void ShortcutsDialog::CancelEvent() {
  close();
}