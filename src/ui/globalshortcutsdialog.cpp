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

#include <QtDebug>
#include <QSettings>
#include <QPushButton>
#include <QKeyEvent>

const char* GlobalShortcutsDialog::kSettingsGroup = "Shortcuts";

GlobalShortcutsDialog::GlobalShortcutsDialog(QWidget* parent)
  : QDialog(parent),
    ui_(new Ui_GlobalShortcutsDialog),
    grabber_(new GlobalShortcutGrabber)
{
  ui_->setupUi(this);
  ui_->shortcut_options->setEnabled(false);

  ui_->list->header()->setResizeMode(QHeaderView::ResizeToContents);

  settings_.beginGroup(kSettingsGroup);

  AddShortcut("play", tr("Play"));
  AddShortcut("pause", tr("Pause"));
  AddShortcut("play_pause", tr("Play/Pause"), QKeySequence(Qt::Key_MediaPlay));
  AddShortcut("stop", tr("Stop"), QKeySequence(Qt::Key_MediaStop));
  AddShortcut("stop_after", tr("Stop playing after current track"));
  AddShortcut("next_track", tr("Next track"), QKeySequence(Qt::Key_MediaNext));
  AddShortcut("prev_track", tr("Previous track"), QKeySequence(Qt::Key_MediaPrevious));
  AddShortcut("inc_volume", tr("Increase volume"));
  AddShortcut("dec_volume", tr("Decrease volume"));
  AddShortcut("mute", tr("Mute"));
  AddShortcut("seek_forward", tr("Seek forward"));
  AddShortcut("seek_backward", tr("Seek backward"));

  ui_->list->sortItems(0, Qt::AscendingOrder);

  connect(ui_->button_box->button(QDialogButtonBox::Reset), SIGNAL(clicked()), SLOT(ResetAll()));
  connect(ui_->list, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), SLOT(ItemClicked(QTreeWidgetItem*)));
  connect(ui_->radio_none, SIGNAL(clicked()), SLOT(NoneClicked()));
  connect(ui_->radio_default, SIGNAL(clicked()), SLOT(DefaultClicked()));
  connect(ui_->radio_custom, SIGNAL(clicked()), SLOT(ChangeClicked()));
  connect(ui_->change, SIGNAL(clicked()), SLOT(ChangeClicked()));
}

GlobalShortcutsDialog::~GlobalShortcutsDialog() {
  delete ui_;
}

void GlobalShortcutsDialog::AddShortcut(const QString& id, const QString& name,
                                        const QKeySequence& default_key) {
  Shortcut s;
  s.id = id;
  s.name = name;
  s.default_key = default_key;
  s.key = QKeySequence::fromString(settings_.value(id, default_key.toString()).toString());

  s.item = new QTreeWidgetItem(ui_->list,
      QStringList() << name << s.key.toString(QKeySequence::NativeText));
  s.item->setData(0, Qt::UserRole, id);

  shortcuts_[id] = s;
}

void GlobalShortcutsDialog::ResetAll() {
  foreach (const QString& id, shortcuts_.keys()) {
    SetShortcut(id, shortcuts_[id].default_key);
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
  foreach (const QString& id, shortcuts_.keys()) {
    settings_.setValue(id, shortcuts_[id].key.toString());
  }
}

void GlobalShortcutsDialog::ItemClicked(QTreeWidgetItem* item) {
  current_id_ = item->data(0, Qt::UserRole).toString();
  Shortcut& shortcut = shortcuts_[current_id_];

  // Enable options
  ui_->shortcut_options->setEnabled(true);
  ui_->shortcut_options->setTitle(tr("Shortcut for %1").arg(shortcut.name));

  if (shortcut.key == shortcut.default_key)
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
  SetShortcut(current_id_, shortcuts_[current_id_].default_key);
}

void GlobalShortcutsDialog::ChangeClicked() {
  QKeySequence key = grabber_->GetKey(shortcuts_[current_id_].name);
  if (key.isEmpty())
    return;

  ui_->radio_custom->setChecked(true);
  SetShortcut(current_id_, key);
}
