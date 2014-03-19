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

#ifndef SUGGESTIONWIDGET_H
#define SUGGESTIONWIDGET_H

#include <QWidget>

class Ui_SuggestionWidget;

class SuggestionWidget : public QWidget {
  Q_OBJECT

 public:
  SuggestionWidget(const QIcon& search_icon, QWidget* parent = nullptr);
  ~SuggestionWidget();

  bool eventFilter(QObject* object, QEvent* event);

 public slots:
  void SetText(const QString& text);

signals:
  void SuggestionClicked(const QString& query);

 private:
  Ui_SuggestionWidget* ui_;
};

#endif  // SUGGESTIONWIDGET_H
