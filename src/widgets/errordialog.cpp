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

#include "errordialog.h"
#include "ui_errordialog.h"

ErrorDialog::ErrorDialog(QWidget* parent)
    : QDialog(parent), ui_(new Ui_ErrorDialog) {
  ui_->setupUi(this);

  QIcon warning_icon(style()->standardIcon(QStyle::SP_MessageBoxWarning));
  QPixmap warning_pixmap(warning_icon.pixmap(48));

  QPalette messages_palette(ui_->messages->palette());
  messages_palette.setColor(QPalette::Base,
                            messages_palette.color(QPalette::Background));

  ui_->messages->setPalette(messages_palette);
  ui_->icon->setPixmap(warning_pixmap);
}

ErrorDialog::~ErrorDialog() { delete ui_; }

void ErrorDialog::ShowMessage(const QString& message) {
  current_messages_ << message;
  UpdateContent();

  show();
  raise();
  activateWindow();
}

void ErrorDialog::hideEvent(QHideEvent*) {
  current_messages_.clear();
  UpdateContent();
}

void ErrorDialog::UpdateContent() {
  QString html;
  foreach(const QString & message, current_messages_) {
    if (!html.isEmpty()) html += "<hr/>";
    html += Qt::escape(message);
  }
  ui_->messages->setHtml(html);
}
