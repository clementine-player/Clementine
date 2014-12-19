/* This file is part of Clementine.
   Copyright 2012, Arnaud Bienner <arnaud.bienner@gmail.com>
   Copyright 2014, John Maguire <john.maguire@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>
   Copyright 2014, Maltsev Vlad <shedwardx@gmail.com>

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

#ifndef INTERNET_CORE_SEARCHBOXWIDGET_H_
#define INTERNET_CORE_SEARCHBOXWIDGET_H_

#include <QWidget>

class InternetService;
class DidYouMean;
class Ui_SearchBoxWidget;

class QActionGroup;
class QMenu;

class SearchBoxWidget : public QWidget {
  Q_OBJECT

 public:
  explicit SearchBoxWidget(InternetService* service);
  ~SearchBoxWidget();

  DidYouMean* did_you_mean() { return did_you_mean_; }

 signals:
  void TextChanged(const QString& text);

 public slots:
  void FocusOnFilter(QKeyEvent* e);
  void SetText(const QString& text);

 protected:
  void keyReleaseEvent(QKeyEvent* e);

 private:
  InternetService* service_;
  Ui_SearchBoxWidget* ui_;
  QMenu* menu_;
  DidYouMean* did_you_mean_;
};

#endif  // INTERNET_CORE_SEARCHBOXWIDGET_H_
