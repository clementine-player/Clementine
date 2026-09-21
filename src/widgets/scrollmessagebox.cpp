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

#include "scrollmessagebox.h"
#include <QStyle>
#include <QApplication>
#include <QPushButton>
#include <QGridLayout>
#include <QtGui/QScrollArea>

ScrollMessageBox::ScrollMessageBox(QMessageBox::Icon icon, QString const& title, 
  QString const& text, QDialogButtonBox::StandardButtons buttons /*= QDialogButtonBox::Ok*/, 
  QWidget* parent /*= 0*/) :
  QDialog(parent, Qt::Dialog | Qt::WindowTitleHint | Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint) {

  QLabel *iconLabel;
  QScrollArea *scroll;

  label = new QLabel;
  label->setTextInteractionFlags(Qt::TextInteractionFlags(style()->styleHint(QStyle::SH_MessageBox_TextInteractionFlags, 0, this)));
  label->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
  label->setOpenExternalLinks(true);
  label->setContentsMargins(2, 0, 0, 0);
  label->setIndent(9);

  scroll = new QScrollArea(this);
  scroll->setGeometry(QRect(10, 20, 560, 430));
  scroll->setWidget(label);
  scroll->setWidgetResizable(true);

  iconLabel = new QLabel;
  iconLabel->setPixmap(standardIcon((QMessageBox::Icon)icon));
  iconLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  buttonBox = new QDialogButtonBox(buttons);
  buttonBox->setCenterButtons(style()->styleHint(QStyle::SH_MessageBox_CenterButtons, 0, this));
  QObject::connect(buttonBox, SIGNAL(clicked(QAbstractButton*)),
    this, SLOT(handle_buttonClicked(QAbstractButton*)));

  QGridLayout *grid = new QGridLayout;

  grid->addWidget(iconLabel, 0, 0, 2, 1, Qt::AlignTop);
  grid->addWidget(scroll, 0, 1, 1, 1);
  grid->addWidget(buttonBox, 1, 0, 1, 2);
  grid->setSizeConstraint(QLayout::SetNoConstraint);
  setLayout(grid);

  if (!title.isEmpty() || !text.isEmpty()) {
    setWindowTitle(title);
    label->setText(text);
  }

  setModal(true);
}

QPixmap ScrollMessageBox::standardIcon(QMessageBox::Icon icon) {
  QStyle *style = this->style();
  int iconSize = style->pixelMetric(QStyle::PM_MessageBoxIconSize, 0, this);
  QIcon tmpIcon;
  
  switch (icon) {
    case QMessageBox::Information:
      tmpIcon = style->standardIcon(QStyle::SP_MessageBoxInformation, 0, this);
      break;
    case QMessageBox::Warning:
      tmpIcon = style->standardIcon(QStyle::SP_MessageBoxWarning, 0, this);
      break;
    case QMessageBox::Critical:
      tmpIcon = style->standardIcon(QStyle::SP_MessageBoxCritical, 0, this);
      break;
    case QMessageBox::Question:
      tmpIcon = style->standardIcon(QStyle::SP_MessageBoxQuestion, 0, this);
    default:
      break;
  }
  
  if (!tmpIcon.isNull()) {
    return tmpIcon.pixmap(iconSize, iconSize);
  }
  return QPixmap();
}

void ScrollMessageBox::handle_buttonClicked(QAbstractButton *button) {
  int ret = buttonBox->standardButton(button);
  done(ret); 
}

void ScrollMessageBox::setDefaultButton(QPushButton *button) {
  if (!buttonBox->buttons().contains(button))
    return;
  button->setDefault(true);
  button->setFocus();
}

void ScrollMessageBox::setDefaultButton(QDialogButtonBox::StandardButton button) {
  setDefaultButton(buttonBox->button(button));
}

void ScrollMessageBox::showEvent(QShowEvent *e) {
  QDialog::showEvent(e);
}

QDialogButtonBox::StandardButton ScrollMessageBox::critical(QWidget* parent, QString const& title, QString const& text,
  QDialogButtonBox::StandardButtons buttons, QDialogButtonBox::StandardButton defaultButton) {
  ScrollMessageBox box(QMessageBox::Critical, title, text, buttons, parent);
  box.setDefaultButton(defaultButton);
  return static_cast<QDialogButtonBox::StandardButton>(box.exec());
}

QDialogButtonBox::StandardButton ScrollMessageBox::information(QWidget* parent, QString const& title, QString const& text,
  QDialogButtonBox::StandardButtons buttons, QDialogButtonBox::StandardButton defaultButton) {
  ScrollMessageBox box(QMessageBox::Information, title, text, buttons, parent);
  box.setDefaultButton(defaultButton);
  return static_cast<QDialogButtonBox::StandardButton>(box.exec());
}

QDialogButtonBox::StandardButton ScrollMessageBox::question(QWidget* parent, QString const& title, QString const& text,
  QDialogButtonBox::StandardButtons buttons, QDialogButtonBox::StandardButton defaultButton) {
  ScrollMessageBox box(QMessageBox::Question, title, text, buttons, parent);
  box.setDefaultButton(defaultButton);
  return static_cast<QDialogButtonBox::StandardButton>(box.exec());
}

QDialogButtonBox::StandardButton ScrollMessageBox::warning(QWidget* parent, QString const& title, QString const& text,
  QDialogButtonBox::StandardButtons buttons, QDialogButtonBox::StandardButton defaultButton) {
  ScrollMessageBox box(QMessageBox::Warning, title, text, buttons, parent);
  box.setDefaultButton(defaultButton);
  return static_cast<QDialogButtonBox::StandardButton>(box.exec());
}
