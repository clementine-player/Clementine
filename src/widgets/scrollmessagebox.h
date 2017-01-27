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

#ifndef SCROLLMESSAGEBOX_H
#define SCROLLMESSAGEBOX_H

#include <QDialog>
#include <QMessageBox>
#include <QDialogButtonBox>
#include <QLabel>

class ScrollMessageBox : public QDialog {
  Q_OBJECT

public:
  ScrollMessageBox(QMessageBox::Icon icon, QString const& title, QString const& text,
    QDialogButtonBox::StandardButtons buttons = QDialogButtonBox::Ok, QWidget* parent = 0);

  void setDefaultButton(QDialogButtonBox::StandardButton button);

  static QDialogButtonBox::StandardButton critical(QWidget* parent, QString const& title, QString const& text, QDialogButtonBox::StandardButtons buttons = QDialogButtonBox::Ok, QDialogButtonBox::StandardButton defaultButton = QDialogButtonBox::NoButton);
  static QDialogButtonBox::StandardButton information(QWidget* parent, QString const& title, QString const& text, QDialogButtonBox::StandardButtons buttons = QDialogButtonBox::Ok, QDialogButtonBox::StandardButton defaultButton = QDialogButtonBox::NoButton);
  static QDialogButtonBox::StandardButton question(QWidget* parent, QString const& title, QString const& text, QDialogButtonBox::StandardButtons buttons = QDialogButtonBox::Ok, QDialogButtonBox::StandardButton defaultButton = QDialogButtonBox::NoButton);
  static QDialogButtonBox::StandardButton warning(QWidget* parent, QString const& title, QString const& text, QDialogButtonBox::StandardButtons buttons = QDialogButtonBox::Ok, QDialogButtonBox::StandardButton defaultButton = QDialogButtonBox::NoButton);

  void showEvent ( QShowEvent * event ) override;

private:
  QPixmap standardIcon(QMessageBox::Icon icon);
  void setDefaultButton(QPushButton *button);
  void updateSize();
 
  QLabel *label;
  QDialogButtonBox *buttonBox;

private Q_SLOTS:
  void handle_buttonClicked(QAbstractButton *button);
};

#endif  // SCROLLMESSAGEBOX_H
