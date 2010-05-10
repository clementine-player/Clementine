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

#ifndef RADIOVIEWCONTAINER_H
#define RADIOVIEWCONTAINER_H

#include <QWidget>

class RadioModel;
class RadioService;
class RadioView;
class Ui_RadioViewContainer;

class QTimeLine;
class QModelIndex;

class RadioViewContainer : public QWidget {
  Q_OBJECT

 public:
  RadioViewContainer(QWidget* parent = 0);
  ~RadioViewContainer();

  void SetModel(RadioModel* model);

  RadioView* tree() const;

 private slots:
  void Collapsed(const QModelIndex& index);
  void Expanded(const QModelIndex& index);
  void CurrentIndexChanged(const QModelIndex& index);
  void SetFilterHeight(int height);

 private:
  void ServiceChanged(const QModelIndex& index, bool changed_away = false);
  void SetFilterVisible(bool visible);

 private:
  Ui_RadioViewContainer* ui_;
  RadioModel* model_;
  RadioService* current_service_;

  bool filter_visible_;
  QTimeLine* filter_animation_;
};

#endif // RADIOVIEWCONTAINER_H
