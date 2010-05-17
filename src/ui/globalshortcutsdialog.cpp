/* This file is part of Clementine.

   Clementine is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Clementine is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Clementine.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "globalshortcutgrabber.h"
#include "globalshortcutsdialog.h"
#include "ui_globalshortcutsdialog.h"
#include "core/globalshortcuts.h"

#include <QtDebug>
#include <QSettings>
#include <QPushButton>
#include <QKeyEvent>
#include <QProcess>
#include <QMessageBox>

GlobalShortcutsDialog::GlobalShortcutsDialog(GlobalShortcuts* manager,
                                             QWidget* parent)
  : QDialog(parent),
    ui_(new Ui_GlobalShortcutsDialog),
    manager_(manager),
    grabber_(new GlobalShortcutGrabber)
{
  ui_->setupUi(this);
  ui_->shortcut_options->setEnabled(false);
  ui_->list->header()->setResizeMode(QHeaderView::ResizeToContents);

  if (!manager->IsGsdAvailable()) {
    ui_->gnome_container->hide();
  }

  settings_.beginGroup(GlobalShortcuts::kSettingsGroup);

  foreach (const GlobalShortcuts::Shortcut& s, manager_->shortcuts().values()) {
    Shortcut shortcut;
    shortcut.s = s;
    shortcut.key = s.action->shortcut();
    shortcut.item = new QTreeWidgetItem(ui_->list,
        QStringList() << s.action->text()
                      << s.action->shortcut().toString(QKeySequence::NativeText));
    shortcut.item->setData(0, Qt::UserRole, s.id);
    shortcuts_[s.id] = shortcut;
  }

  ui_->list->sortItems(0, Qt::AscendingOrder);

  connect(ui_->button_box->button(QDialogButtonBox::Reset), SIGNAL(clicked()), SLOT(ResetAll()));
  connect(ui_->list, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), SLOT(ItemClicked(QTreeWidgetItem*)));
  connect(ui_->radio_none, SIGNAL(clicked()), SLOT(NoneClicked()));
  connect(ui_->radio_default, SIGNAL(clicked()), SLOT(DefaultClicked()));
  connect(ui_->radio_custom, SIGNAL(clicked()), SLOT(ChangeClicked()));
  connect(ui_->change, SIGNAL(clicked()), SLOT(ChangeClicked()));
  connect(ui_->gnome_open, SIGNAL(clicked()), SLOT(OpenGnomeKeybindingProperties()));

  ItemClicked(ui_->list->topLevelItem(0));
}

GlobalShortcutsDialog::~GlobalShortcutsDialog() {
  delete ui_;
}

void GlobalShortcutsDialog::showEvent(QShowEvent*) {
  foreach (const Shortcut& s, shortcuts_.values()) {
    SetShortcut(s.s.id, s.s.action->shortcut());
  }

  bool use_gnome = settings_.value("use_gnome", true).toBool();
  if (ui_->gnome_container->isVisible()) {
    ui_->gnome_checkbox->setChecked(use_gnome);
  }
}

void GlobalShortcutsDialog::ResetAll() {
  foreach (const QString& id, shortcuts_.keys()) {
    SetShortcut(id, shortcuts_[id].s.default_key);
  }
}

void GlobalShortcutsDialog::SetShortcut(const QString& id, const QKeySequence& key) {
  Shortcut& shortcut = shortcuts_[id];

  shortcut.key = key;
  shortcut.item->setText(1, key.toString(QKeySequence::NativeText));
}

void GlobalShortcutsDialog::accept() {
  Save();
  QDialog::accept();
}

void GlobalShortcutsDialog::Save() {
  foreach (const Shortcut& s, shortcuts_.values()) {
    s.s.action->setShortcut(s.key);
    settings_.setValue(s.s.id, s.key.toString());
  }

  settings_.setValue("use_gnome", ui_->gnome_checkbox->isChecked());
}

void GlobalShortcutsDialog::ItemClicked(QTreeWidgetItem* item) {
  current_id_ = item->data(0, Qt::UserRole).toString();
  Shortcut& shortcut = shortcuts_[current_id_];

  // Enable options
  ui_->shortcut_options->setEnabled(true);
  ui_->shortcut_options->setTitle(tr("Shortcut for %1").arg(shortcut.s.action->text()));

  if (shortcut.key == shortcut.s.default_key)
    ui_->radio_default->setChecked(true);
  else if (shortcut.key.isEmpty())
    ui_->radio_none->setChecked(true);
  else
    ui_->radio_custom->setChecked(true);
}

void GlobalShortcutsDialog::NoneClicked() {
  SetShortcut(current_id_, QKeySequence());
}

void GlobalShortcutsDialog::DefaultClicked() {
  SetShortcut(current_id_, shortcuts_[current_id_].s.default_key);
}

void GlobalShortcutsDialog::ChangeClicked() {
  QKeySequence key = grabber_->GetKey(shortcuts_[current_id_].s.action->text());
  if (key.isEmpty())
    return;

  ui_->radio_custom->setChecked(true);
  SetShortcut(current_id_, key);
}

void GlobalShortcutsDialog::OpenGnomeKeybindingProperties() {
  if (!QProcess::startDetached("gnome-keybinding-properties")) {
    QMessageBox::warning(this, "Error",
        tr("The \"%1\" command could not be started.")
        .arg("gnome-keybinding-properties"));
  }
}
