/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>

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

#include "suggestionwidget.h"

#include <QMouseEvent>

#include "ui/iconloader.h"
#include "ui_suggestionwidget.h"

SuggestionWidget::SuggestionWidget(const QIcon& search_icon, QWidget* parent)
    : QWidget(parent), ui_(new Ui_SuggestionWidget) {
  ui_->setupUi(this);

  ui_->icon->setPixmap(search_icon.pixmap(16));
  ui_->name->installEventFilter(this);
}

SuggestionWidget::~SuggestionWidget() { delete ui_; }

void SuggestionWidget::SetText(const QString& text) {
  ui_->name->setText(text);
}

bool SuggestionWidget::eventFilter(QObject* object, QEvent* event) {
  if (object != ui_->name) {
    return QWidget::eventFilter(object, event);
  }

  QFont font(ui_->name->font());

  switch (event->type()) {
    case QEvent::Enter:
      font.setUnderline(true);
      ui_->name->setFont(font);
      break;

    case QEvent::Leave:
      font.setUnderline(false);
      ui_->name->setFont(font);
      break;

    case QEvent::MouseButtonRelease: {
      QMouseEvent* e = static_cast<QMouseEvent*>(event);
      if (e->button() == Qt::LeftButton) {
        QString text = ui_->name->text();
        text.replace(QRegExp("\\W"), " ");

        emit SuggestionClicked(text.simplified());
      }
      break;
    }

    default:
      return false;
  }

  return true;
}
