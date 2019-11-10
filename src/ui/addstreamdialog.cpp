/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

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

#include "addstreamdialog.h"
#include "ui_addstreamdialog.h"
#include "internet/core/internetmodel.h"
#include "internet/internetradio/savedradio.h"

#include <QSettings>
#include <QUrl>
#include <QPushButton>
#include <QtDebug>

const char* AddStreamDialog::kSettingsGroup = "AddStreamDialog";

AddStreamDialog::AddStreamDialog(QWidget* parent)
    : QDialog(parent), ui_(new Ui_AddStreamDialog), saved_radio_(nullptr) {
  ui_->setupUi(this);

  connect(ui_->url, SIGNAL(textChanged(QString)), SLOT(TextChanged(QString)));
  TextChanged(QString());

  // Restore settings
  QSettings s;
  s.beginGroup(kSettingsGroup);
  ui_->save->setChecked(s.value("save", true).toBool());
  ui_->url->setText(s.value("url").toString());
  ui_->name->setText(s.value("name").toString());
}

AddStreamDialog::~AddStreamDialog() { delete ui_; }

QUrl AddStreamDialog::url() const { return QUrl(ui_->url->text()); }

QString AddStreamDialog::name() const { return ui_->name->text(); }

void AddStreamDialog::set_name(const QString& name) {
  ui_->name->setText(name);
}

void AddStreamDialog::set_url(const QUrl& url) {
  ui_->url->setText(url.toString());
}

void AddStreamDialog::set_save_visible(bool visible) {
  ui_->save->setVisible(visible);
  if (!visible) ui_->name_container->setEnabled(true);
}

void AddStreamDialog::accept() {
  if (ui_->save->isChecked() && saved_radio_) {
    saved_radio_->Add(url(), name());
  }

  // Save settings
  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.setValue("save", ui_->save->isChecked());
  s.setValue("url", url().toString());
  s.setValue("name", ui_->name->text());

  QDialog::accept();
}

void AddStreamDialog::TextChanged(const QString& text) {
  // Decide whether the URL is valid
  QUrl url(text);

  bool valid =
      url.isValid() && !url.scheme().isEmpty() && !url.toString().isEmpty();

  ui_->button_box->button(QDialogButtonBox::Ok)->setEnabled(valid);
}

void AddStreamDialog::showEvent(QShowEvent*) {
  ui_->url->setFocus();
  ui_->url->selectAll();
}
