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

#include "addstreamdialog.h"
#include "ui_addstreamdialog.h"
#include "radio/radiomodel.h"
#include "radio/savedradio.h"

#include <QSettings>
#include <QUrl>
#include <QPushButton>
#include <QtDebug>

const char* AddStreamDialog::kSettingsGroup = "AddStreamDialog";

AddStreamDialog::AddStreamDialog(QWidget *parent)
  : QDialog(parent),
    ui_(new Ui_AddStreamDialog),
    saved_radio_(NULL)
{
  ui_->setupUi(this);

  connect(ui_->url, SIGNAL(textChanged(QString)), SLOT(TextChanged(QString)));
  TextChanged(QString::null);

  // Restore settings
  QSettings s;
  s.beginGroup(kSettingsGroup);
  ui_->save->setChecked(s.value("save", true).toBool());
  ui_->url->setText(s.value("url").toString());

  // Connections to the saved streams service
  saved_radio_ = qobject_cast<SavedRadio*>(
      RadioModel::ServiceByName(SavedRadio::kServiceName));

  connect(saved_radio_, SIGNAL(ShowAddStreamDialog()), SLOT(show()));
}

AddStreamDialog::~AddStreamDialog() {
  delete ui_;
}

QUrl AddStreamDialog::url() const {
  return QUrl(ui_->url->text());
}

void AddStreamDialog::accept() {
  if (ui_->save->isChecked()) {
    saved_radio_->Add(url());
  }

  // Save settings
  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.setValue("save", ui_->save->isChecked());
  s.setValue("url", url().toString());

  QDialog::accept();
}

void AddStreamDialog::TextChanged(const QString &text) {
  // Decide whether the URL is valid
  QUrl url(text);

  bool valid = url.isValid() &&
               !url.scheme().isEmpty() &&
               !url.toString().isEmpty();

  ui_->button_box->button(QDialogButtonBox::Ok)->setEnabled(valid);
}

