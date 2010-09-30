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

#ifndef SLIMBUTTONBOX_H
#define SLIMBUTTONBOX_H

#include <QIcon>
#include <QList>
#include <QWidget>

class QPushButton;

class SlimButtonBox : public QWidget {
  Q_OBJECT

public:
  SlimButtonBox(QWidget* parent = 0);

  void AddButton(const QString& text, const QIcon& icon = QIcon());
  bool IsAnyButtonChecked() const;
  void SetCurrentButton(int index);

  void SetTabBarBase(bool tab_bar_base);

signals:
  void CurrentChanged(int index);

protected:
  void changeEvent(QEvent* e);
  void paintEvent(QPaintEvent*);

private slots:
  void ButtonClicked();

private:
  void ReloadStylesheet();

private:
  bool tab_bar_base_;
  QList<QPushButton*> buttons_;
};

#endif // SLIMBUTTONBOX_H
