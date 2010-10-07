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

#ifndef COLLAPSIBLEINFOHEADER_H
#define COLLAPSIBLEINFOHEADER_H

#include <QWidget>

class CollapsibleInfoHeader : public QWidget {
  Q_OBJECT

public:
  CollapsibleInfoHeader(QWidget* parent = 0);

  static const int kHeight;

  bool expanded() const { return expanded_; }
  bool hovering() const { return hovering_; }
  const QString& title() const { return title_; }

public slots:
  void SetExpanded(bool expanded);
  void SetTitle(const QString& title);

signals:
  void Expanded();
  void Collapsed();
  void ExpandedToggled(bool expanded);

protected:
  void enterEvent(QEvent*);
  void leaveEvent(QEvent*);
  void paintEvent(QPaintEvent* e);
  void mouseReleaseEvent(QMouseEvent*);

private:
  bool expanded_;
  bool hovering_;
  QString title_;
};

#endif // COLLAPSIBLEINFOHEADER_H
