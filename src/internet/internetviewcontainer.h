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

#ifndef INTERNETVIEWCONTAINER_H
#define INTERNETVIEWCONTAINER_H

#include <QWidget>
#include <QMap>

class Application;
class InternetService;
class InternetView;
class LibraryFilterWidget;
class Ui_InternetViewContainer;

class QTimeLine;
class QModelIndex;

class InternetViewContainer : public QWidget {
  Q_OBJECT

 public:
  InternetViewContainer(QWidget* parent = nullptr);
  ~InternetViewContainer();

  static const int kAnimationDuration;

  void SetApplication(Application* app);

  InternetView* tree() const;

 public slots:
  void ScrollToIndex(const QModelIndex& index);

 private slots:
  void Collapsed(const QModelIndex& index);
  void Expanded(const QModelIndex& index);
  void CurrentIndexChanged(const QModelIndex& index);
  void SetHeaderHeight(int height);

  void FocusOnFilter(QKeyEvent* event);

 private:
  void ServiceChanged(const QModelIndex& index);
  void SetHeaderVisible(QWidget* header, bool visible);

 private:
  Ui_InternetViewContainer* ui_;
  Application* app_;
  InternetService* current_service_;

  QWidget* current_header_;

  struct HeaderData {
    bool visible_;
    QTimeLine* animation_;
  };
  QMap<QWidget*, HeaderData> headers_;
};

#endif  // INTERNETVIEWCONTAINER_H
