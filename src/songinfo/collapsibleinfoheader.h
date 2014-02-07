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

#ifndef COLLAPSIBLEINFOHEADER_H
#define COLLAPSIBLEINFOHEADER_H

#include <QIcon>
#include <QWidget>

class QPropertyAnimation;

class CollapsibleInfoHeader : public QWidget {
  Q_OBJECT
  Q_PROPERTY(float opacity READ opacity WRITE set_opacity);

 public:
  CollapsibleInfoHeader(QWidget* parent = 0);

  static const int kHeight;
  static const int kIconSize;

  bool expanded() const { return expanded_; }
  bool hovering() const { return hovering_; }
  const QString& title() const { return title_; }
  const QIcon& icon() const { return icon_; }

  float opacity() const { return opacity_; }
  void set_opacity(float opacity);

 public slots:
  void SetExpanded(bool expanded);
  void SetTitle(const QString& title);
  void SetIcon(const QIcon& icon);

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
  QIcon icon_;

  QPropertyAnimation* animation_;
  float opacity_;
};

#endif  // COLLAPSIBLEINFOHEADER_H
