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

#include "loginstatewidget.h"
#include "ui_loginstatewidget.h"
#include "ui/iconloader.h"

#include <QKeyEvent>

LoginStateWidget::LoginStateWidget(QWidget* parent)
  : QWidget(parent),
    ui_(new Ui_LoginStateWidget)
{
  ui_->setupUi(this);
  ui_->signed_in->hide();
  ui_->account_type->hide();
  ui_->busy->hide();

  ui_->sign_out->setIcon(IconLoader::Load("list-remove"));

  QFont bold_font(font());
  bold_font.setBold(true);
  ui_->signed_out_label->setFont(bold_font);

  connect(ui_->sign_out, SIGNAL(clicked()), SLOT(Logout()));
}

LoginStateWidget::~LoginStateWidget() {
  delete ui_;
}

void LoginStateWidget::Logout() {
  SetLoggedIn(LoggedOut);
  emit LogoutClicked();
}

void LoginStateWidget::SetAccountTypeText(const QString& text) {
  ui_->account_type_label->setText(text);
}

void LoginStateWidget::SetAccountTypeVisible(bool visible) {
  ui_->account_type->setVisible(visible);
}

void LoginStateWidget::SetLoggedIn(State state, const QString& account_name) {
  ui_->signed_in->setVisible(state == LoggedIn);
  ui_->signed_out->setVisible(state != LoggedIn);
  ui_->busy->setVisible(state == LoginInProgress);

  if (account_name.isEmpty())
    ui_->signed_in_label->setText("<b>" + tr("You are signed in.") + "</b>");
  else
    ui_->signed_in_label->setText(tr("You are signed in as %1.").arg("<b>" + account_name + "</b>"));

  foreach (QWidget* widget, credential_groups_) {
    widget->setVisible(state != LoggedIn);
    widget->setEnabled(state != LoginInProgress);
  }
}

void LoginStateWidget::HideLoggedInState() {
  ui_->signed_in->hide();
  ui_->signed_out->hide();
}

void LoginStateWidget::AddCredentialField(QWidget* widget) {
  widget->installEventFilter(this);
  credential_fields_ << widget;
}

void LoginStateWidget::AddCredentialGroup(QWidget* widget) {
  credential_groups_ << widget;
}

bool LoginStateWidget::eventFilter(QObject* object, QEvent* event) {
  if (!credential_fields_.contains(object))
    return QWidget::eventFilter(object, event);

  if (event->type() == QEvent::KeyPress) {
    QKeyEvent* key_event = static_cast<QKeyEvent*>(event);
    if (key_event->key() == Qt::Key_Enter ||
        key_event->key() == Qt::Key_Return) {
      emit LoginClicked();
      return true;
    }
  }

  return QWidget::eventFilter(object, event);
}

